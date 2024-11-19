#include <Geode/Geode.hpp>

#include <Geode/modify/GJGameLevel.hpp>
#include <Geode/modify/GameLevelManager.hpp>
#include <Geode/modify/PlayLayer.hpp>

class $modify(ModGJGameLevel, GJGameLevel) {
	void updateLeaderboard() {
		auto gm = GameManager::sharedState();
		auto glm = GameLevelManager::sharedState();

		auto leaderboardType = gm->getIntGameVariable("0098");
		auto leaderboardMode = gm->getIntGameVariable("0164");

		// using weekly should cut down the amount of data fetched
		// might help mobile players on cell service but probably doesn't matter

		// gd only updates whatever stat is fetched, so we have to call getLevelLeaderboard twice for platformer levels
		glm->getLevelLeaderboard(static_cast<GJGameLevel*>(this), LevelLeaderboardType::Weekly, LevelLeaderboardMode::Time);
		if (isPlatformer()) glm->getLevelLeaderboard(static_cast<GJGameLevel*>(this), LevelLeaderboardType::Weekly, LevelLeaderboardMode::Points);

		// getLevelLeaderboard automatically sets the mode and type to whatever was passed into it, so we have to reset it
		gm->setIntGameVariable("0098", leaderboardType);
		gm->setIntGameVariable("0164", leaderboardMode);
	}

	$override
	void savePercentage(int percent, bool isPracticeMode, int clicks, int attempts, bool isChkValid) {
		auto oldPercent = m_normalPercent.value();
		GJGameLevel::savePercentage(percent, isPracticeMode, clicks, attempts, isChkValid);

		// we don't want this running when the percent is 100%, because this function runs before coins are calculated
		if (percent >= 100 || percent <= oldPercent) return;

		updateLeaderboard();
	}
};

class $modify(PlayLayer) {
	// since savePercentage runs before coins are calculated, on level complete we hook this function instead
	$override
	void levelComplete() {
		auto oldPercent = m_level->m_normalPercent.value();
		auto oldBestTime = m_level->m_bestTime;
		PlayLayer::levelComplete();

		bool isNewBestTime = m_level->m_bestTime < oldBestTime;
		if (oldBestTime == 0) isNewBestTime = true;

		// i couldn't find a good way to determine if the player collected more coins,
		// so we always update the leaderboard when beating non-platformer levels
		// but this shouldn't be a big deal, since you don't beat levels often enough to cause issues like rate limiting

		if (m_level->isPlatformer() && !isNewBestTime) return;

		static_cast<ModGJGameLevel*>(m_level)->updateLeaderboard();
	}
};

class $modify(GameLevelManager) {
	$override
	void handleIt(bool idk, gd::string fetchedData, gd::string tag, GJHttpType httpType) {
		GameLevelManager::handleIt(idk, fetchedData, tag, httpType);
		if (httpType != GJHttpType::GetLevelLeaderboard) return;

		// m_storedLevels holds all cached level-related data (levels, comments, leaderboards, etc)
		// m_timerDict holds timestamps for when the data was last fetched. deleting these forces a refetch

		// doing this prevents the leaderboard from erroneously showing "1 second ago" despite beating the level earlier than that

		m_storedLevels->removeObjectForKey(tag);
		m_timerDict->removeObjectForKey(tag);
	}
};