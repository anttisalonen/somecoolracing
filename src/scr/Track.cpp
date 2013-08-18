#include <cassert>
#include <cmath>

#include <stdexcept>
#include <fstream>
#include <sstream>

#include <jsoncpp/json/json.h>

#include "common/Math.h"

#include "Track.h"

using namespace Common;

StraightTrackSegment::StraightTrackSegment(const Common::Vector2& startpos,
		const Common::Vector2& dir,
		float len, float width)
	: mStartPos(startpos),
	mDir(dir),
	mLength(len),
	mWidth(width)
{
	assert(mDir.length() > 0);
	mDir.normalize();
	mEndPos = mStartPos + mDir * mLength;
}

bool StraightTrackSegment::onTrack(const Common::Vector2& pos) const
{
	// actually extends the segment at ends by up to half mWidth
	bool ret = Math::pointToSegmentDistance(mStartPos,
			mEndPos,
			pos) < mWidth * 0.5;
	return ret;
}

std::vector<Common::Vector2> StraightTrackSegment::getTriangleStrip() const
{
	Vector2 v1 = mStartPos + Math::rotate2D(mDir, HALF_PI) * mWidth * 0.5f;
	Vector2 v2 = mStartPos - Math::rotate2D(mDir, HALF_PI) * mWidth * 0.5f;
	Vector2 v3 = mEndPos + Math::rotate2D(mDir, HALF_PI) * mWidth * 0.5f;
	Vector2 v4 = mEndPos - Math::rotate2D(mDir, HALF_PI) * mWidth * 0.5f;
	return {v1, v2, v3, v4};
}

Common::Vector2 StraightTrackSegment::getEndPosition() const
{
	return mEndPos;
}

float StraightTrackSegment::getLength() const
{
	return mLength;
}


CurveSegment::CurveSegment(const Common::Vector2& startpos,
		const Common::Vector2& dir,
		const Common::Vector2& endpos,
		const Common::Vector2& enddir,
		float width)
	: mStartPos(startpos),
	mEndPos(endpos),
	mDir(dir),
	mEndDir(enddir),
	mWidth(width)
{
	assert(mDir.length() > 0);
	assert(mEndDir.length() > 0);
	mDir.normalize();
	mEndDir.normalize();

	// Second order Bezier curve
	// mStartPos = p0
	// mEndPos = p2
	// We'll calculate p1 now as the intersection of the two
	// lines p0 + mDir and p2 - mEndDir.

	mP1 = Common::Math::lineLineIntersection2D(
			mStartPos, mStartPos + mDir,
			mEndPos, mEndPos - mEndDir);

	if(mP1.null()) {
		throw std::runtime_error("Cannot construct a curve!");
	}

	// Calculate an estimation of the curve length
	float len = 0.0f;
	for(int i = 0; i < 10; i++) {
		auto p1 = pointOnCurve(i / 10.0);
		auto p2 = pointOnCurve((i + 1) / 10.0);
		len += p1.distance(p2);
	}
	mNumApproxSegments = 5 + pow(len, 0.5f);

	// Create lines that estimate the curve for onTrack() query.
	for(size_t i = 0; i < mNumApproxSegments; i++) {
		auto pt = pointOnCurve(i / (float)(mNumApproxSegments - 1));
		mApproximations.push_back(pt);
	}
}

bool CurveSegment::onTrack(const Common::Vector2& pos) const
{
	for(size_t i = 0; i < mApproximations.size() - 1; i++) {
		float this_dist = Math::pointToSegmentDistance(
				mApproximations[i],
				mApproximations[i + 1],
				pos);
		if(this_dist < mWidth * 0.5f)
			return true;
	}

	return false;
}

std::vector<Common::Vector2> CurveSegment::getTriangleStrip() const
{
	std::vector<Common::Vector2> ret;
	for(size_t i = 0; i < mApproximations.size(); i++) {
		float t = i / (float)(mNumApproxSegments - 1);
		auto pc = pointOnCurve(t);

		// Calculate normal of the curve from the
		// direction (tangent, derivative) of the curve.
		auto dir = directionOnCurve(t);
		Vector2 v1 = pc + Math::rotate2D(dir, HALF_PI) * mWidth * 0.5f;
		Vector2 v2 = pc - Math::rotate2D(dir, HALF_PI) * mWidth * 0.5f;
		ret.push_back(v1);
		ret.push_back(v2);
	}
	return ret;
}

Common::Vector2 CurveSegment::getEndPosition() const
{
	return mEndPos;
}

float CurveSegment::getLength() const
{
	float len = 0.0f;
	for(size_t i = 0; i < mApproximations.size() - 1; i++) {
		len += mApproximations[i].distance(mApproximations[i + 1]);
	}
	return len;
}

Common::Vector2 CurveSegment::pointOnCurve(float t) const
{
	assert(t >= 0.0f && t <= 1.001f);
	float ti = 1.0f - t;
	return mStartPos * ti * ti + mP1 * 2.0f * ti * t + mEndPos * t * t;
}

Common::Vector2 CurveSegment::directionOnCurve(float t) const
{
	// derivative
	assert(t >= 0.0f && t <= 1.001f);
	float ti = 1.0f - t;
	return ((mP1 - mStartPos) * 2.0f * ti + (mEndPos - mP1) * 2.0f * t).normalized();
}

Track::Track(const TrackConfig* tc)
{
	float width = tc->Width;
	const Vector2 startpos(0.0f, 0.0f);
	Vector2 pos = startpos;
	const Vector2 startdir(1.0f, 0.0f);
	Vector2 dir = startdir;
	for(const auto& ts : tc->Segments) {
		switch(ts.Type) {
			case TrackConfig::TSType::Straight:
				{
					if(!ts.Info.StraightInfo.Length)
						throw std::runtime_error("Straight segment without length");
					auto seg = new StraightTrackSegment(pos,
							dir,
							ts.Info.StraightInfo.Length,
							width);
					pos = seg->getEndPosition();
					mSegments.push_back(seg);
					stretchLimits(pos);
				}
				break;

			case TrackConfig::TSType::Curve:
				{
					Vector2 endpos;
					auto enddir = Vector2(ts.Info.CurveInfo.endDirection_x,
							ts.Info.CurveInfo.endDirection_y);
					if(enddir.null()) {
						throw std::runtime_error("Curve segment without end direction");
					}
					enddir.normalize();
					if(!ts.Info.CurveInfo.endOffset_x &&
							!ts.Info.CurveInfo.endOffset_y) {
						endpos.x = ts.Info.CurveInfo.endPosition_x;
						endpos.y = ts.Info.CurveInfo.endPosition_y;
					} else {
						endpos.x = pos.x + ts.Info.CurveInfo.endOffset_x;
						endpos.y = pos.y + ts.Info.CurveInfo.endOffset_y;
					}
					auto seg = new CurveSegment(pos,
							dir,
							endpos,
							enddir,
							width);
					pos = seg->getEndPosition();
					dir = enddir;
					mSegments.push_back(seg);
					stretchLimits(seg->pointOnCurve(0.5f));
					stretchLimits(pos);
				}
				break;
		}
	}

	if(pos.distance(startpos) > 0.00001f) {
		if(fabs(dir.angleTo(startdir)) < 0.00001f) {
			std::cout << "Finishing track with a straight segment.\n";
			auto seg = new StraightTrackSegment(pos,
					dir,
					pos.distance(startpos),
					width);
			mSegments.push_back(seg);
		} else if(fabs(dir.angleTo(startdir)) < HALF_PI) {
			std::cout << "Finishing track with a curve segment.\n";
			auto seg = new CurveSegment(pos,
					dir,
					startpos,
					startdir,
					width);
			mSegments.push_back(seg);
		} else {
			std::stringstream err;
			err << "Invalid track - it ends at " << pos << ", facing " << dir << ".\n";
			throw std::runtime_error(err.str());
		}
	}

	{
		float len = 0.0f;
		for(const auto& s : mSegments)
			len += s->getLength();
		std::cout << "Track length: " << len << " m\n";
	}
}

Track::~Track()
{
	for(auto s : mSegments)
		delete s;
}

const std::vector<TrackSegment*>& Track::getTrackSegments() const
{
	return mSegments;
}

bool Track::onTrack(const Common::Vector2& pos) const
{
	// TODO: use quad tree
	for(auto s : mSegments)
		if(s->onTrack(pos))
			return true;

	return false;
}

void Track::getLimits(Common::Vector2& bl, Common::Vector2& tr) const
{
	bl = mBottomLeft;
	tr = mTopRight;
}

void Track::stretchLimits(const Common::Vector2& trackpos)
{
	mBottomLeft.x = std::min<float>(mBottomLeft.x, trackpos.x - 200.0f);
	mBottomLeft.y = std::min<float>(mBottomLeft.y, trackpos.y - 200.0f);
	mTopRight.x   = std::max<float>(mTopRight.x,   trackpos.x + 200.0f);
	mTopRight.y   = std::max<float>(mTopRight.y,   trackpos.y + 200.0f);
}

TrackConfig Track::readTrackConfig(const char* filename)
{
	Json::Reader reader;
	Json::Value root;

	std::ifstream input(filename, std::ifstream::binary);
	bool parsingSuccessful = reader.parse(input, root, false);
	if (!parsingSuccessful) {
		throw std::runtime_error(reader.getFormatedErrorMessages());
	}

	TrackConfig tc;
	tc.Width = root["width"].asDouble();
	for(const auto& seg : root["segments"]) {
		TrackConfig::TSInfo info;
		std::string type = seg["type"].asString();
		if(seg["type"] == "straight") {
			info.Type = TrackConfig::TSType::Straight;
			info.Info.StraightInfo.Length = seg["length"].asDouble();
		} else if(seg["type"] == "curve") {
			info.Type = TrackConfig::TSType::Curve;
			info.Info.CurveInfo.endOffset_x = seg["endOffset"][0u].asDouble();
			info.Info.CurveInfo.endOffset_y = seg["endOffset"][1u].asDouble();
			info.Info.CurveInfo.endPosition_x = seg["endPosition"][0u].asDouble();
			info.Info.CurveInfo.endPosition_y = seg["endPosition"][1u].asDouble();
			info.Info.CurveInfo.endDirection_x = seg["endDirection"][0u].asDouble();
			info.Info.CurveInfo.endDirection_y = seg["endDirection"][1u].asDouble();
		} else {
			throw std::runtime_error("Unknown segment type!");
		}
		tc.Segments.push_back(info);
	}

	return tc;
}


