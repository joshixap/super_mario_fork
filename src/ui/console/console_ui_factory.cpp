#include "console_ui_factory.hpp"
#include <algorithm>

using biv::ConsoleUIFactory;

ConsoleUIFactory::ConsoleUIFactory(Game* game) : UIFactory(game) {
	create_game_map();

	game->set_on_collisionable_killed([this](biv::Collisionable* obj) {
		// 1) убрать объект с карты (чтобы не рисовался)
		if (auto ui_obj = dynamic_cast<biv::ConsoleUIObject*>(obj)) {
			game_map->remove_obj(ui_obj);
		}

		// 2) удалить объект из контейнеров фабрики + освободить память

		if (auto money = dynamic_cast<biv::ConsoleMoney*>(obj)) {
			moneys.erase(std::remove(moneys.begin(), moneys.end(), money), moneys.end());
			pending_delete.push_back(money);
			return;
		}

		if (auto e = dynamic_cast<biv::ConsoleFlyableEnemy*>(obj)) {
			flyable_enemies.erase(std::remove(flyable_enemies.begin(), flyable_enemies.end(), e), flyable_enemies.end());
			pending_delete.push_back(e);
			return;
		}

		if (auto e = dynamic_cast<biv::ConsoleMovableEnemy*>(obj)) {
			movable_enemies.erase(std::remove(movable_enemies.begin(), movable_enemies.end(), e), movable_enemies.end());
			pending_delete.push_back(e);
			return;
		}

		if (auto e = dynamic_cast<biv::ConsoleJumpableEnemy*>(obj)) {
			jumpable_enemies.erase(std::remove(jumpable_enemies.begin(), jumpable_enemies.end(), e), jumpable_enemies.end());
			pending_delete.push_back(e);
			return;
		}

		if (auto e = dynamic_cast<biv::ConsoleEnemy*>(obj)) {
			enemies.erase(std::remove(enemies.begin(), enemies.end(), e), enemies.end());
			pending_delete.push_back(e);
			return;
		}

		if (auto b = dynamic_cast<biv::ConsoleFullBox*>(obj)) {
			return;
		}

		// Box/Ship/Mario сюда не должны попадать при обычной игре (они не kill()-ятся так).
	});
}

void ConsoleUIFactory::clear_data() {
	// 0) сначала удалить всех, кто уже в очереди на удаление
	flush_pending_delete();

	// 1) убрать ссылки из Game/GameMap (они не владеют памятью)
	game->remove_objs();
	game_map->remove_objs();

	// 2) удалить Марио (если есть)
	delete mario;
	mario = nullptr;

	// 3) удалить всё, что создавала фабрика и что ещё осталось "живым"

	for (auto* x : boxes) delete x;
	for (auto* x : full_boxes) delete x;
	for (auto* x : ships) delete x;

	for (auto* x : enemies) delete x;
	for (auto* x : movable_enemies) delete x;
	for (auto* x : flyable_enemies) delete x;
	for (auto* x : jumpable_enemies) delete x;

	for (auto* x : moneys) delete x;

	// 4) очистить контейнеры
	boxes.clear();
	full_boxes.clear();
	ships.clear();

	enemies.clear();
	movable_enemies.clear();
	flyable_enemies.clear();
	jumpable_enemies.clear();

	moneys.clear();

	// 5) на всякий случай: очередь должна быть пустая
	pending_delete.clear();
}

void ConsoleUIFactory::create_box(
	const Coord& top_left, const int width, const int height
) {
	ConsoleBox* box = new ConsoleBox(top_left, width, height);
	boxes.push_back(box);
	game->add_map_movable(box);
	game->add_static_obj(box);
	game_map->add_obj(box);
}

void ConsoleUIFactory::tick_cleanup() {
	flush_pending_delete();
}

void ConsoleUIFactory::flush_pending_delete() {
	// удаляем всё, что отложили на delete
	for (Collisionable* obj : pending_delete) {
		// ВАЖНО: delete по реальному типу.
		// Тут safe, потому что мы удаляем именно те объекты,
		// которые создавала фабрика через new (ConsoleMoney, ConsoleFlyableEnemy, ...)

		// Поскольку у нас нет виртуального деструктора в Collisionable (возможно),
		// лучше удалять через dynamic_cast к конкретным типам.

		if (auto money = dynamic_cast<ConsoleMoney*>(obj)) { delete money; continue; }
		if (auto e = dynamic_cast<ConsoleFlyableEnemy*>(obj)) { delete e; continue; }
		if (auto e = dynamic_cast<ConsoleMovableEnemy*>(obj)) { delete e; continue; }
		if (auto e = dynamic_cast<ConsoleJumpableEnemy*>(obj)) { delete e; continue; }
		if (auto e = dynamic_cast<ConsoleEnemy*>(obj)) { delete e; continue; }
		if (auto b = dynamic_cast<ConsoleFullBox*>(obj)) { delete b; continue; }

		// На всякий: если сюда попадёт что-то неожиданное, лучше не удалять молча.
		// (можно оставить пустым или добавить лог)
	}

	pending_delete.clear();
}

void ConsoleUIFactory::create_enemy(
	const Coord& top_left, const int width, const int height
) {
	ConsoleEnemy* enemy = new ConsoleEnemy(top_left, width, height);
	enemies.push_back(enemy);
	game->add_map_movable(enemy);
	game->add_movable(enemy);
	game->add_collisionable(enemy);
	game_map->add_obj(enemy);
}

void ConsoleUIFactory::create_movable_enemy(
	const Coord& top_left, const int width, const int height
) {
	ConsoleMovableEnemy* enemy = new ConsoleMovableEnemy(top_left, width, height);
	movable_enemies.push_back(enemy);
	game->add_map_movable(enemy);
	game->add_movable(enemy);
	game->add_collisionable(enemy);
	game_map->add_obj(enemy);
}

void ConsoleUIFactory::create_flyable_enemy(
	const Coord& top_left, const int width, const int height
) {
	ConsoleFlyableEnemy* enemy = new ConsoleFlyableEnemy(top_left, width, height);
	flyable_enemies.push_back(enemy);
	game->add_map_movable(enemy);
	game->add_movable(enemy);
	game->add_collisionable(enemy);
	game_map->add_obj(enemy);
}

void ConsoleUIFactory::create_jumpable_enemy(
	const Coord& top_left, const int width, const int height
) {
	ConsoleJumpableEnemy* enemy = new ConsoleJumpableEnemy(top_left, width, height);
	jumpable_enemies.push_back(enemy);
	game->add_map_movable(enemy);
	game->add_movable(enemy);
	game->add_collisionable(enemy);
	game_map->add_obj(enemy);
}

void ConsoleUIFactory::create_full_box(
	const Coord& top_left, const int width, const int height
) {
	ConsoleFullBox* full_box = new ConsoleFullBox(top_left, width, height, this);
	full_boxes.push_back(full_box);
	game->add_collisionable(full_box);
	game->add_map_movable(full_box);
	game->add_static_obj(full_box);
	game_map->add_obj(full_box);
}

void ConsoleUIFactory::create_mario(
	const Coord& top_left, const int width, const int height
) {
	game->remove_collisionable(mario);
	game->remove_movable(mario);
	game->remove_mario();
	game_map->remove_obj(mario);
	delete mario;
	mario = nullptr;
	
	mario = new ConsoleMario(top_left, width, height);
	game->add_collisionable(mario);
	game->add_movable(mario);
	game->add_mario(mario);
	game_map->add_obj(mario);
}

void ConsoleUIFactory::create_money(
	const Coord& top_left, const int width, const int height
) {
	ConsoleMoney* money = new ConsoleMoney(top_left, width, height);
	moneys.push_back(money);
	game->add_map_movable(money);
	game->add_movable(money);
	game->add_collisionable(money);
	game_map->add_obj(money);
}

void ConsoleUIFactory::create_ship(
	const Coord& top_left, const int width, const int height
) {
	ConsoleShip* ship = new ConsoleShip(top_left, width, height);
	ships.push_back(ship);
	game->add_map_movable(ship);
	game->add_static_obj(ship);
	game_map->add_obj(ship);
}

biv::GameMap* ConsoleUIFactory::get_game_map() {
	return game_map;
}

biv::Mario* ConsoleUIFactory::get_mario() {
	return mario;
}

ConsoleUIFactory::~ConsoleUIFactory() {
	clear_data();
	delete game_map;
	game_map = nullptr;
}

// ----------------------------------------------------------------------------
// 									PRIVATE
// ----------------------------------------------------------------------------
void ConsoleUIFactory::create_game_map() {
	game_map = new ConsoleGameMap(40, 160);
}
