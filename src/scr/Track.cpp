#include <cassert>
#include <cmath>

#include <stdexcept>

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
	std::cout << len << "\t" << mNumApproxSegments << "\n";

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

Track::Track()
{
	const float width = 15.0f;
	auto s1 = new StraightTrackSegment(Vector2(0.0f, 0.0f),
				Vector2(1.0f, 0.0f),
				200.0f, width);
	auto s1end = s1->getEndPosition();

	auto s2end = s1end + Vector2(200.0f, 600.0f);
	auto s2enddir = Vector2(-1.0f, 2.0f).normalized();
	auto s2 = new CurveSegment(s1end,
				Vector2(1.0f, 0.0f),
				s2end,
				s2enddir,
				width);

	auto s3end = s2end + Vector2(-600.0f, 0.0f);
	auto s3enddir = Vector2(-3.0f, -1.0f).normalized();
	auto s3 = new CurveSegment(s2end, s2enddir, s3end, s3enddir, width);

	auto s4 = new StraightTrackSegment(s3end, s3enddir, 300.0f, width);
	auto s4end = s4->getEndPosition();

	auto s5end = Vector2(-600.0f, 0.0f);
	auto s5enddir = Vector2(1.0f, 0.0f);
	auto s5 = new CurveSegment(s4end, s3enddir, s5end, s5enddir, width);

	auto s6 = new StraightTrackSegment(s5end, s5enddir, -s5end.x, width);

	mSegments.push_back(s1);
	mSegments.push_back(s2);
	mSegments.push_back(s3);
	mSegments.push_back(s4);
	mSegments.push_back(s5);
	mSegments.push_back(s6);
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
	bl.x = -2000.0;
	bl.y = -2000.0;
	tr.x = 2000.0;
	tr.y = 2000.0;
}

