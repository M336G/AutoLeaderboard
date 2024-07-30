#include <Geode/modify/LevelInfoLayer.hpp>

class $modify(LevelInfoLayer) {
	bool init(GJGameLevel* level, bool challenge) {
		if (!LevelInfoLayer::init(level, challenge)) return false;
		
		if (level->m_normalPercent != 0) {
			auto leaderboard = LevelLeaderboard::create(level, LevelLeaderboardType::Global, LevelLeaderboardMode::Time);
			leaderboard->show();
			// Laggier if setVisible is used
			leaderboard->setPositionY(-2000);

			// Wait for the scores to be updated before closing
			leaderboard->updateUserScoreFinished();
			leaderboard->onClose(nullptr);

			if (level->m_levelLength == 5) { // Platformer check
				// Same thing but for platformer points
				auto leaderboard = LevelLeaderboard::create(level, LevelLeaderboardType::Global, LevelLeaderboardMode::Points);
				leaderboard->show();
				leaderboard->setPositionY(-2000);

				leaderboard->updateUserScoreFinished();
				leaderboard->onClose(nullptr);
			}
		}
		return true;
	}
};