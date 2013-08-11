#ifndef SCR_TRACK_H
#define SCR_TRACK_H

#include <vector>
#include <array>

#include "common/Vector2.h"

class TrackSegment {
	public:
		virtual ~TrackSegment() { }

		// functions needed by the game engine
		virtual bool onTrack(const Common::Vector2& pos) const = 0;

		// functions needed by graphics
		virtual std::vector<Common::Vector2> getTriangleStrip() const = 0;

		// functions needed by track creation
		virtual Common::Vector2 getEndPosition() const = 0;
};

class StraightTrackSegment : public TrackSegment {
	public:
		StraightTrackSegment(const Common::Vector2& startpos,
				const Common::Vector2& dir,
				float len, float width);
		virtual bool onTrack(const Common::Vector2& pos) const override;
		virtual std::vector<Common::Vector2> getTriangleStrip() const override;
		virtual Common::Vector2 getEndPosition() const override;

	private:
		Common::Vector2 mStartPos;
		Common::Vector2 mEndPos;
		Common::Vector2 mDir;
		float mLength;
		float mWidth;
};

class CurveSegment : public TrackSegment {
	public:
		CurveSegment(const Common::Vector2& startpos,
				const Common::Vector2& dir,
				const Common::Vector2& endpos,
				const Common::Vector2& enddir,
				float width);
		virtual bool onTrack(const Common::Vector2& pos) const override;
		virtual std::vector<Common::Vector2> getTriangleStrip() const override;
		virtual Common::Vector2 getEndPosition() const override;

	private:
		// 0 <= t <= 1
		Common::Vector2 pointOnCurve(float t) const;
		Common::Vector2 directionOnCurve(float t) const;

		Common::Vector2 mStartPos;
		Common::Vector2 mEndPos;
		Common::Vector2 mDir;
		Common::Vector2 mEndDir;
		Common::Vector2 mP1;
		float mWidth;

		// The size depends on the samples used in
		// getTriangleStrip
		std::array<Common::Vector2, 11> mApproximations;
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

