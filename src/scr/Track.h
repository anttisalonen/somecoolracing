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
		virtual float getLength() const = 0;
};

class StraightTrackSegment : public TrackSegment {
	public:
		StraightTrackSegment(const Common::Vector2& startpos,
				const Common::Vector2& dir,
				float len, float width);
		virtual bool onTrack(const Common::Vector2& pos) const override;
		virtual std::vector<Common::Vector2> getTriangleStrip() const override;
		virtual Common::Vector2 getEndPosition() const override;
		virtual float getLength() const override;

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
		virtual float getLength() const override;

		// 0 <= t <= 1
		Common::Vector2 pointOnCurve(float t) const;
		Common::Vector2 directionOnCurve(float t) const;

	private:
		Common::Vector2 mStartPos;
		Common::Vector2 mEndPos;
		Common::Vector2 mDir;
		Common::Vector2 mEndDir;
		Common::Vector2 mP1;
		float mWidth;

		std::vector<Common::Vector2> mApproximations;
		int mNumApproxSegments;
};

struct TrackConfig {
	enum class TSType {
		Straight,
		Curve
	};

	struct TSInfo {
		TSType Type;
		union {
			struct {
				float Length;
			} StraightInfo;
			struct {
				float endOffset_x;
				float endOffset_y;
				float endDirection_x;
				float endDirection_y;
				float endPosition_x;
				float endPosition_y;
			} CurveInfo;
		} Info;
	};

	float Width;
	std::vector<TSInfo> Segments;
};

class Track {
	public:
		Track(const TrackConfig* tc);
		~Track();
		const std::vector<TrackSegment*>& getTrackSegments() const;
		bool onTrack(const Common::Vector2& pos) const;
		void getLimits(Common::Vector2& bl, Common::Vector2& tr) const;

		static TrackConfig readTrackConfig(const char* filename);

	private:
		void stretchLimits(const Common::Vector2& trackpos);

		std::vector<TrackSegment*> mSegments;
		Common::Vector2 mBottomLeft;
		Common::Vector2 mTopRight;
};

#endif

