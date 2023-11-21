#pragma once
#include <memory>
#include <map>
#include <functional>
#include <DxLib.h>
#include "ActorBase.h"
class AnimationController;
class Collider;

class Player : public ActorBase
{

public:

	// �X�s�[�h
	static constexpr float SPEED_MOVE = 5.0f;
	static constexpr float SPEED_RUN = 10.0f;

	// ��]�����܂ł̎���
	static constexpr float TIME_ROT = 1.0f;

	// �W�����v��
	static constexpr float POW_JUMP = 35.0f;
	// �W�����v��t����
	static constexpr float TIME_JUMP_IN = 0.5f;

	static constexpr float PLAYER_SHADOW_SIZE = 50.0f;		// �e�̑傫��
	static constexpr float PLAYER_SHADOW_HEIGHT = 100.0f;

	// ���
	enum class STATE
	{
		NONE,
		PLAY,
		WARP_RESERVE,
		WARP_MOVE,
		DEAD,
		VICTORY,
		END
	};

	// �A�j���[�V�������
	enum class ANIM_TYPE
	{
		IDLE,
		RUN,
		FAST_RUN,
		JUMP,
		WARP_PAUSE,
		FLY,
		FALLING,
		VICTORY
	};

	// �R���X�g���N�^
	Player(void);

	// �f�X�g���N�^
	~Player(void);

	void Init(void) override;
	void Update(void) override;
	void Draw(void) override;

	// �Փ˔���ɗp������R���C�_����
	void AddCollider(std::weak_ptr<Collider> collider);
	void ClearCollider(void);

private:

	// �A�j���[�V����
	std::unique_ptr<AnimationController> animationController_;

	// ��ԊǗ�
	STATE state_;

	// ��ԊǗ�(��ԑJ�ڎ���������)
	std::map<STATE, std::function<void(void)>> stateChanges_;

	// ��ԊǗ�(�X�V�X�e�b�v)
	std::function<void(void)> stateUpdate_;
	
	// �ړ��X�s�[�h
	float speed_;

	// �ړ�����
	VECTOR moveDir_;

	// �ړ���
	VECTOR movePow_;

	// �ړ���̍��W
	VECTOR movedPos_;

	// �W�����v��
	VECTOR jumpPow_;

	// �Փ˔���ɗp������R���C�_
	std::vector<std::weak_ptr<Collider>> colliders_;

	// �Փ˃`�F�b�N
	VECTOR gravHitPosDown_;
	VECTOR gravHitPosUp_;

	// ��]
	Quaternion playerRotY_;
	Quaternion goalQuaRot_;
	float stepRotTime_;

	// �W�����v����
	bool isJump_;

	// �W�����v�̓��͎�t����
	float stepJump_;

	// �ۉe
	int imgShadow_;

	void InitAnimation(void);

	// ��ԑJ��
	void ChangeState(STATE state);
	void ChangeStateNone(void);
	void ChangeStatePlay(void);

	// �X�V�X�e�b�v
	void UpdateNone(void);
	void UpdatePlay(void);

	// �`��n
	void DrawDebug(void);

	// ���� transform_.pos = movedPos_;
	void ProcessMove(void);

	// ��]
	void SetGoalRotate(double rotRad);
	void Rotate(void);

	// �Փ˔���
	void Collision(void);
	void CollisionGravity(void);

	// �ړ��ʂ̌v�Z
	void CalcGravityPow(void);

	void ProcessJump(void);

	// ���n���[�V�����I��
	bool IsEndLanding(void);

	void DrawShadow(void);

};
