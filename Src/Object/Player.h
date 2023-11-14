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

private:

	// �A�j���[�V����
	std::unique_ptr<AnimationController> animationController_;

	// ��ԊǗ�
	STATE state_;
	// ��ԊǗ�(��ԑJ�ڎ���������)
	std::map<STATE, std::function<void(void)>> stateChanges_;
	// ��ԊǗ�(�X�V�X�e�b�v)
	std::function<void(void)> stateUpdate_;
	
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

};
