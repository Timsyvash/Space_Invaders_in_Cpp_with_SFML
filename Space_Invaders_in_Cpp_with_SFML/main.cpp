#include <SFML/Graphics.hpp>
#include <vector>
#include <cstdlib>

using namespace std;
using namespace sf;

const unsigned short window_width = 800;
const unsigned short window_height = 800;

const char* path_background = "Images/background.png";
const char* path_player = "Images/Player.png";
const char* path_enemy = "Images/Enemy.png";

struct Laser {
	RectangleShape shape;
	float speed;
};

class Player {
public:
	Sprite sprite_player;
	vector<Laser> lasers;
	float x, y, width, height, speed, laserSpeed;
	float shootTimer, shootDelay;

	Player(Texture& texture_player) {
		sprite_player.setTexture(texture_player);
		width = 60.0f;
		height = 37.0f;
		speed = 0.4f;
		laserSpeed = 0.8f;
		shootDelay = 250.0f;

		Reset();
	}

	void Reset() {
		x = window_width / 2.0f - width / 2.0f;
		y = window_height - height - 10.0f;
		sprite_player.setPosition(x, y);
		shootTimer = 0.0f;
		lasers.clear();
	}

	void Update(float time) {
		if (Keyboard::isKeyPressed(Keyboard::A) || Keyboard::isKeyPressed(Keyboard::Left)) {
			x -= speed * time;
		}
		if (Keyboard::isKeyPressed(Keyboard::D) || Keyboard::isKeyPressed(Keyboard::Right)) {
			x += speed * time;
		}

		if (x < 0) x = 0;
		if (x > window_width - width) x = window_width - width;

		sprite_player.setPosition(x, y);

		shootTimer += time;

		for (size_t i = 0; i < lasers.size(); ) {
			lasers[i].shape.move(0, -lasers[i].speed * time);

			if (lasers[i].shape.getPosition().y < -20.0f) {
				lasers.erase(lasers.begin() + i);
			}
			else {
				i++;
			}
		}
	}

	void Shooter() {
		if (Keyboard::isKeyPressed(Keyboard::Space) && shootTimer >= shootDelay) {
			Laser newLaser;
			newLaser.shape.setSize(Vector2f(4.f, 20.f));
			newLaser.shape.setFillColor(Color::Blue);
			newLaser.shape.setPosition(x + width / 2.0f - 2.0f, y);
			newLaser.speed = laserSpeed;

			lasers.push_back(newLaser);
			shootTimer = 0.0f;
		}
	}
};

class Enemy {
public:
	float speed, shootTimer, shootDelay, laserSpeed;
	bool movingRight;
	vector<Laser> lasers;
	vector<Sprite> enemies;
	Texture* texturePtr;

	short rows, cols;
	float spacing_x, spacing_y, offset_x, offset_y;

	Enemy(Texture& texture_enemy) {
		texturePtr = &texture_enemy;
		speed = 0.15f;
		laserSpeed = 0.6f;
		shootDelay = 1000.0f;

		rows = 4;
		cols = 8;
		spacing_x = 90.0f;
		spacing_y = 110.0f;
		offset_x = 50.0f;
		offset_y = 50.0f;

		Reset();
	}

	void Reset() {
		movingRight = true;
		shootTimer = 0.0f;
		lasers.clear();
		enemies.clear();

		Sprite enemyPrototype;
		enemyPrototype.setTexture(*texturePtr);

		for (int r = 0; r < rows; ++r) {
			for (int c = 0; c < cols; ++c) {
				Sprite enemy = enemyPrototype;
				enemy.setPosition(offset_x + c * spacing_x, offset_y + r * spacing_y);
				enemies.push_back(enemy);
			}
		}
	}

	void Update(float time) {
		if (!enemies.empty()) {
			bool touch_edge = false;

			for (const auto& e : enemies) {
				if (movingRight && e.getPosition().x + e.getGlobalBounds().width >= window_width) {
					touch_edge = true;
					break;
				}
				if (!movingRight && e.getPosition().x <= 0) {
					touch_edge = true;
					break;
				}
			}

			if (touch_edge) {
				movingRight = !movingRight;
				for (auto& e : enemies) {
					e.move(0.f, 15.f);
				}
			}
			else {
				float step = (movingRight ? speed : -speed) * time;
				for (auto& e : enemies) {
					e.move(step, 0.f);
				}
			}
		}

		shootTimer += time;

		for (size_t i = 0; i < lasers.size(); ) {
			lasers[i].shape.move(0, lasers[i].speed * time);

			if (lasers[i].shape.getPosition().y > window_height + 20.0f) {
				lasers.erase(lasers.begin() + i);
			}
			else {
				i++;
			}
		}
	}

	void Shooter() {
		if (!enemies.empty() && shootTimer >= shootDelay) {
			int randomIndex = rand() % enemies.size();
			Vector2f enemyPos = enemies[randomIndex].getPosition();
			FloatRect enemyBounds = enemies[randomIndex].getGlobalBounds();

			Laser newLaser;
			newLaser.shape.setSize(Vector2f(4.f, 20.f));
			newLaser.shape.setFillColor(Color::Red);
			newLaser.shape.setPosition(enemyPos.x + enemyBounds.width / 2.0f - 2.0f, enemyPos.y + enemyBounds.height);
			newLaser.speed = laserSpeed;

			lasers.push_back(newLaser);
			shootTimer = 0.0f;
		}
	}
};

class Game {
public:
	Sprite sprite_background;
	Game(Texture& texture_background) {
		sprite_background.setTexture(texture_background);
		sprite_background.setPosition(0, 0);
	}

	void Collision(Player& player, Enemy& enemyManager, bool& isGameOver) {
		for (size_t i = 0; i < player.lasers.size(); ) {
			bool hit = false;
			for (size_t j = 0; j < enemyManager.enemies.size(); ++j) {
				if (player.lasers[i].shape.getGlobalBounds().intersects(enemyManager.enemies[j].getGlobalBounds())) {
					player.lasers.erase(player.lasers.begin() + i);
					enemyManager.enemies.erase(enemyManager.enemies.begin() + j);
					hit = true;
					break;
				}
			}
			if (!hit) {
				i++;
			}
		}

		for (size_t i = 0; i < enemyManager.lasers.size(); ) {
			if (enemyManager.lasers[i].shape.getGlobalBounds().intersects(player.sprite_player.getGlobalBounds())) {
				enemyManager.lasers.erase(enemyManager.lasers.begin() + i);
				isGameOver = true;
				break;
			}
			else {
				i++;
			}
		}

		for (const auto& e : enemyManager.enemies) {
			if (e.getGlobalBounds().intersects(player.sprite_player.getGlobalBounds())) {
				isGameOver = true;
				break;
			}
		}

		// 4. НОВЕ: Лазери гравця->Лазери ворога(Збивання пострілів у повітрі)
		for (size_t i = 0; i < player.lasers.size(); ) {
			bool laserHit = false;
			for (size_t j = 0; j < enemyManager.lasers.size(); ++j) {
				if (player.lasers[i].shape.getGlobalBounds().intersects(enemyManager.lasers[j].shape.getGlobalBounds())) {
					// Знищуємо обидва лазери
					player.lasers.erase(player.lasers.begin() + i);
					enemyManager.lasers.erase(enemyManager.lasers.begin() + j);
					laserHit = true;
					break;
				}
			}
			if (!laserHit) {
				i++;
			}
		}
	}
};

int main() {
	RenderWindow window(VideoMode(window_width, window_height), "Space Invaders");

	Texture texture_background;
	texture_background.loadFromFile(path_background);
	Game game(texture_background);

	Texture texture_player;
	texture_player.loadFromFile(path_player);
	Player player(texture_player);

	Texture texture_enemy;
	texture_enemy.loadFromFile(path_enemy);
	Enemy enemy(texture_enemy);

	// Перевірка та завантаження шрифту
	Font font;
	bool fontLoaded = font.loadFromFile("Font/times.ttf"); // Покладіть файл arial.ttf біля виконуваного файлу

	Text pauseText, gameOverText, winText;
	if (fontLoaded) {
		pauseText.setFont(font);
		pauseText.setString("PAUSED");
		pauseText.setCharacterSize(50);
		pauseText.setFillColor(Color::Yellow);
		pauseText.setPosition(window_width / 2.0f - pauseText.getGlobalBounds().width / 2.0f, window_height / 2.0f - 30.0f);

		gameOverText.setFont(font);
		gameOverText.setString("GAME OVER\nPress R to Restart");
		gameOverText.setCharacterSize(40);
		gameOverText.setFillColor(Color::Yellow);
		gameOverText.setStyle(Text::Bold);
		gameOverText.setPosition(window_width / 2.0f - gameOverText.getGlobalBounds().width / 2.0f, window_height / 2.0f - 50.0f);

		winText.setFont(font);
		winText.setString("WIN GAME\nPress R to Restart");
		winText.setCharacterSize(40);
		winText.setFillColor(Color::Yellow);
		winText.setStyle(Text::Bold);
		winText.setPosition(window_width / 2.0f - winText.getGlobalBounds().width / 2.0f, window_height / 2.0f - 50.0f);
	}

	// Запасний варіант: напівпрозорий червоний оверлей
	RectangleShape gameOverOverlay(Vector2f(window_width, window_height));
	gameOverOverlay.setFillColor(Color(255, 0, 0, 100));

	Clock clock;
	bool isGameOver = false, isPause = false, isWin = false;

	while (window.isOpen()) {
		Event event;
		float time = clock.getElapsedTime().asMicroseconds();
		clock.restart();
		time = time / 800;

		while (window.pollEvent(event)) {
			if (event.type == Event::Closed)
				window.close();

			if (event.type == Event::KeyPressed) {
				if (event.key.code == Keyboard::P && !isGameOver) {
					isPause = !isPause;
				}
				if ((event.key.code == Keyboard::R && isGameOver) || isWin) {
					player.Reset();
					enemy.Reset();
					isGameOver = false;
					isPause = false;
					isWin = false;
				}

				if (enemy.enemies.empty()) {
					isWin = true;
				}
			}
		}

		if (!isGameOver && !isPause && !isWin) {
			player.Update(time);
			player.Shooter();
			enemy.Update(time);
			enemy.Shooter();

			game.Collision(player, enemy, isGameOver);
		}

		window.clear();
		window.draw(game.sprite_background);

		for (const auto& laser : player.lasers) {
			window.draw(laser.shape);
		}
		for (const auto& laser : enemy.lasers) {
			window.draw(laser.shape);
		}

		window.draw(player.sprite_player);

		for (const auto& e : enemy.enemies) {
			window.draw(e);
		}

		if (isGameOver && fontLoaded) {
			window.draw(gameOverOverlay); // Накладання червоного фільтра при поразці
			if (fontLoaded) {
				window.draw(gameOverText);
			}
		}
		else if (isPause && fontLoaded) {
			window.draw(pauseText);
		}
		else if (isWin && fontLoaded) {
			window.draw(winText);
		}

		window.display();
	}

	return 0;
}