#pragma once
#include <DxLib.h>
#include "../Object/Common/Transform.h"

class ShotBase
{

public:

	// 爆発エフェクトの大きさ
	static constexpr float BLAST_SCALE = 10.0f;

	// 状態
	enum class STATE
	{
		NONE,
		SHOT,
		BLAST,
		END,
	};

	// コンストラクタ・Init・Createの区別がややこしいので、
	// Init無し

	ShotBase(void);

	~ShotBase(void);

	/// <summary>
	/// 弾の生成
	/// </summary>
	/// <param name="birthPos">初期座標</param>
	/// <param name="dir">進行方向</param>
	void Create(VECTOR birthPos, VECTOR dir);

	void Update(void);
	void Draw(void);
	void Release(void);

	// 座標の取得
	VECTOR GetPos(void) const;

	// 座標の設定
	void SetPos(VECTOR pos);

	// 弾の速度
	float GetSpeed(void) const;

	// 生存時間
	float GetTimeAlive(void) const;

	// 球体当たり判定用の半径を取得
	float GetCollisionRadius(void) const;

	// 弾を爆発させる
	void Blast(void);

	ShotBase::STATE GetState(void);

protected:

	// モデル制御の基本情報
	Transform transform_;

	// 状態
	STATE state_;

	// 進行方向
	VECTOR dir_;

	// 弾の速度
	float speed_;

	// 生存時間
	float timeAlive_;
	float stepAlive_;

	float gravityPow_;

	// 当たり判定の半径
	float collisionRadius_;

	// 爆発エフェクト
	int effectBlastResId_;
	int effectBlastPlayId_;

	// パラメータ設定
	void SetParam(void);

	// 状態別更新処理
	void UpdateShot(void);
	void UpdateBlast(void);

	// 状態遷移
	void ChangeState(STATE state);

	// 生存チェック
	void CheckAlive(void);

	// 移動処理
	void Move(void);

	// 爆発エフェクトを再生
	void PlayBlastEffect(void);

};
