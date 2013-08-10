#ifndef SCR_TRACK_H
#define SCR_TRACK_H

#include <vector>

#include "common/Vector2.h"

class TrackSegment {
	public:
		virtual ~TrackSegment() { }

		// functions needed by the game engine
		virtual bool onTrack(const Common::Vector2& pos) const = 0;

		// functions needed by graphics
		virtual std::vector<Common::Vector2> getTriangleStrip() const = 0;
};

class StraightTrackSegment : public TrackSegment {
	public:
		StraightTrackSegment(const Common::Vector2& startpos,
				const Common::Vector2& dir,
				float len, float width);
		virtual bool onTrack(const Common::Vector2& pos) const override;
		virtual std::vector<Common::Vector2> getTriangleStrip() const override;

	private:
		Common::Vector2 mStartPos;
		Common::Vector2 mDir;
		float mLength;
		float mWidth;
};

class Track {
	public:
		Track();
		~Track();
		const std::vector<TrackSegment*>& getTrackSegments() const;
		bool onTrack(const Common::Vector2& pos) const;
		void getLimits(Common::Vector2& bl, Common::Vector2& tr) const;

	private:
		std::vector<TrackSegment*> mSegments;
};

#endif

