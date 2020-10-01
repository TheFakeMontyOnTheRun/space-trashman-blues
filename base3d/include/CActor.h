#ifndef ACTOR_H
#define ACTOR_H

typedef char ActorView;

struct CActor {
	enum EDirection mDirection;
	struct Vec2i mPosition;
	ActorView mView;
};
#endif
