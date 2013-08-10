#include <cassert>
#include <cmath>

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
	assert(fabs(dir.length() - 1.0f) < 0.001f);
}

bool StraightTrackSegment::onTrack(const Common::Vector2& pos) const
{
	// actually extends the segment at ends by up to half mWidth
	return Math::pointToLineDistance(mStartPos,
			mStartPos + mDir * mLength,
			pos) < mWidth * 0.5;
}

std::vector<Common::Vector2> StraightTrackSegment::getTriangleStrip() const
{
	Vector2 v1 = mStartPos + Math::rotate2D(mDir, HALF_PI) * mWidth * 0.5f;
	Vector2 v2 = mStartPos - Math::rotate2D(mDir, HALF_PI) * mWidth * 0.5f;
	Vector2 end = mStartPos + mDir * mLength;
	Vector2 v3 = end + Math::rotate2D(mDir, HALF_PI) * mWidth * 0.5f;
	Vector2 v4 = end - Math::rotate2D(mDir, HALF_PI) * mWidth * 0.5f;
	return {v1, v2, v3, v4};
}

Track::Track()
{
	mSegments.push_back(new StraightTrackSegment(Vector2(1.0f, 0.0f),
				Vector2(1.0f, 0.0f),
				50.0f, 5.0f));
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
	bl.x = -100.0;
	bl.y = -100.0;
	tr.x = 100.0;
	tr.y = 100.0;
}

