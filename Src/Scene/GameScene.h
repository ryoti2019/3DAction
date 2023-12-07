#pragma once
#include <memory>
#include "SceneBase.h"
class Stage;
class SkyDome;
class Player;
class Tank;

class GameScene : public SceneBase
{

public:
	
	// コンストラクタ
	GameScene(void);

	// デストラクタ
	~GameScene(void);

	void Init(void) override;
	void Update(void) override;
	void Draw(void) override;

private:

	// ステージ
	std::unique_ptr<Stage> stage_;

	// スカイドーム
	std::unique_ptr<SkyDome> skyDome_;

	// プレイヤー
	std::shared_ptr<Player> player_;

	// タンク
	std::shared_ptr<Tank> tank_;

};
