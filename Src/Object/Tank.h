#pragma once
#include "Common/Transform.h"
#include "ActorBase.h"

class Collider;
class ShotBase;
class Capsule;

class Tank : public ActorBase
{
public:

	// 回転完了までの時間
	static constexpr float TIME_ROT = 1.0f;

	// 弾の発射間隔
	static constexpr float TIME_DELAY_SHOT = 1.0f;

	// コンストラクタ
	Tank(void);

	// デストラクタ
	~Tank(void);

	void Init(void)override;
	void Update(void)override;
	void Draw(void)override;

	// 衝突判定に用いられるコライダ制御
	void AddCollider(std::weak_ptr<Collider> collider);
	void ClearCollider(void);

private:

	// 弾
	std::vector<ShotBase*> shots_;

	// 衝突判定に用いられるコライダ
	std::vector<std::weak_ptr<Collider>> colliders_;

	std::unique_ptr<Capsule> capsule_;

	// 衝突チェック
	VECTOR gravHitPosDown_;

	VECTOR gravHitPosUp_;

	// ボディの基本情報
	Transform transformBody_;

	// 右車輪の基本情報
	Transform transformRWheel_;

	// 左車輪の基本情報
	Transform transformLWheel_;

	// 砲身の基本情報
	Transform transformBarrel_;

	Quaternion playerRotY_;
	Quaternion goalQuaRot_;

	float stepRotTime_;

	// 土台の回転動作蓄積用
	VECTOR localRotAddBody_;

	// 車輪の回転動作蓄積用
	VECTOR localRotAddRWheel_;
	VECTOR localRotAddLWheel_;

	// 砲身の回転動作蓄積用
	VECTOR localRotAddBarrel_;

	// 移動後の座標
	VECTOR movedPos_;

	// ジャンプ量
	VECTOR jumpPow_;

	// 移動量
	VECTOR movePow_;

	// 足元衝突している地面のポリゴン量の法線
	VECTOR hitNormal_;

	// 足元衝突判定している地面ポリゴンの位置
	VECTOR hitPos_;

	float delayShot_;

	// 傾斜の方向
	VECTOR slopeDir_;

	// 傾斜移動を開始する傾斜角度(デグリー)
	float slopeAngleDeg_;

	// 傾斜移動時の移動量
	VECTOR slopePow_;

	// 衝突判定
	void Collision(void);
	void CollisionGravity(void);

	void CollisionCapsule(void);

	void CalcSlope(void);

	// 移動量の計算
	void CalcGravityPow(void);

	// 回転
	void SetGoalRotate(double rotRad);
	void Rotate(void);

	// 親(戦艦)との回転と位置の同期
	void SyncParent(Transform& transform, VECTOR addAxis, VECTOR localPos);

	// 移動操作
	void ProcessMove(void);

	// 発射操作
	void ProcessShot(void);

	void DrawDebug(void);

	void CreateShot(void);

};