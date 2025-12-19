#include "JumpableEnemy.hpp"

#include "map_movable.hpp"

using biv::JumpableEnemy;

JumpableEnemy::JumpableEnemy(const Coord& top_left, const int width, const int height) 
	: RectMapMovableAdapter(top_left, width, height) {
	vspeed = 0;
	hspeed = 0; // Stays in place
	jump_timer = JUMP_PERIOD;
}

biv::Rect JumpableEnemy::get_rect() const noexcept {
	return {top_left, width, height};
}

biv::Speed JumpableEnemy::get_speed() const noexcept {
	return {vspeed, hspeed};
}

void JumpableEnemy::move_vertically() noexcept {
	// Apply gravity and move vertically
	if (vspeed < MAX_V_SPEED) {
		vspeed += V_ACCELERATION;
	}
	top_left.y += vspeed;
}

void JumpableEnemy::process_horizontal_static_collision(Rect* obj) noexcept {
	// No horizontal movement for jumping enemies
}

void JumpableEnemy::process_mario_collision(Collisionable* mario) noexcept {
	if (mario->get_speed().v > 0 && mario->get_speed().v != V_ACCELERATION) {
		kill();
	} else {
		mario->kill();
	}
}

void JumpableEnemy::process_vertical_static_collision(Rect* obj) noexcept {
	// Reset vertical speed when landing
	if (vspeed > 0) {
		top_left.y -= vspeed;
		vspeed = 0;
	}
	
	// Update jump timer every frame when grounded (vspeed == 0)
	if (vspeed == 0) {
		update_jump();
	}
}

void JumpableEnemy::update_jump() noexcept {
	if (vspeed == 0) {
		jump_timer--;
		if (jump_timer <= 0) {
			jump();
			jump_timer = JUMP_PERIOD;
		}
	}
}
