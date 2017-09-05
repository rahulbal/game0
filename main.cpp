#include "Draw.hpp"
#include "GL.hpp"

#include <SDL.h>
#include <glm/glm.hpp>

#include <chrono>
#include <iostream>
#include <cstdio>

static const int DEFAULT_NUM_SHEEP = 5;
static const int STOPPED = 0;
static const int RUNNING = 1;

typedef class Animal {
public:
	glm::vec2 pos;
} Animal;

typedef struct Sheep : public Animal {
public:
	bool col_det;
	glm::vec2 dir;
	Animal* last_col;

	Sheep() : col_det(false), dir(0.0f, 0.0f), last_col(nullptr) { }
} Sheep;

static const bool arr[10][7] = 
		{ 
			{ true, true, true, true, true, true, false },
			{ false, true, true, false, false, false, false },
			{ true, true, false, true, true, false, true },
			{ true, true, true, true, false, false, true },
			{ false, true, true, false, false, true, true },
			{ true, false, true, true, false, true, true },
			{ true, false, true, true, true, true, true },
			{ true, true, true, false, false, false, false },
			{ true, true, true, true, true, true, true },
			{ true, true, true, true, false, true, true} 
		};

void draw_D7(float posx, float posy, bool a, bool b, bool c, bool d, bool e, bool f, bool g) {
	Draw draw;
	if (d) {
		draw.add_rectangle(glm::vec2(posx, posy),
			glm::vec2(0.05f + posx, 0.01f + posy), glm::u8vec4(0xff, 0xff, 0xff, 0xff));
	}
	if (c) {
		draw.add_rectangle(glm::vec2(0.04f + posx, posy),
			glm::vec2(0.05f + posx, 0.05f + posy), glm::u8vec4(0xff, 0xff, 0xff, 0xff));
	}
	if (b) {
		draw.add_rectangle(glm::vec2(0.04f + posx, 0.04f + posy),
			glm::vec2(0.05f + posx, 0.10f + posy), glm::u8vec4(0xff, 0xff, 0xff, 0xff));
	}
	if (a) {
		draw.add_rectangle(glm::vec2(posx, 0.09f + posy),
			glm::vec2(0.05f + posx, 0.10f + posy), glm::u8vec4(0xff, 0xff, 0xff, 0xff));
	}
	if (e) {
		draw.add_rectangle(glm::vec2(posx, posy),
			glm::vec2(0.01f + posx, 0.05f + posy), glm::u8vec4(0xff, 0xff, 0xff, 0xff));
	}
	if (f) {
		draw.add_rectangle(glm::vec2(posx, 0.04f + posy),
			glm::vec2(0.01f + posx, 0.10f + posy), glm::u8vec4(0xff, 0xff, 0xff, 0xff));
	}
	if (g) {
		draw.add_rectangle(glm::vec2(posx, 0.04f + posy),
			glm::vec2(0.05f + posx, 0.05f + posy), glm::u8vec4(0xff, 0xff, 0xff, 0xff));
	}
	draw.draw();
}

int main(int argc, char **argv) {
	//Configuration:
	struct {
		std::string title = "Game0: Tennis For One";
		glm::uvec2 size = glm::uvec2(640, 480);
	} config;

	//------------  initialization ------------

	//Initialize SDL library:
	SDL_Init(SDL_INIT_VIDEO);

	//Ask for an OpenGL context version 3.3, core profile, enable debug:
	SDL_GL_ResetAttributes();
	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

	//create window:
	SDL_Window *window = SDL_CreateWindow(
		config.title.c_str(),
		SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		config.size.x, config.size.y,
		SDL_WINDOW_OPENGL /*| SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI*/
	);

	if (!window) {
		std::cerr << "Error creating SDL window: " << SDL_GetError() << std::endl;
		return 1;
	}

	//Create OpenGL context:
	SDL_GLContext context = SDL_GL_CreateContext(window);

	if (!context) {
		SDL_DestroyWindow(window);
		std::cerr << "Error creating OpenGL context: " << SDL_GetError() << std::endl;
		return 1;
	}

	#ifdef _WIN32
	//On windows, load OpenGL extensions:
	if (!init_gl_shims()) {
		std::cerr << "ERROR: failed to initialize shims." << std::endl;
		return 1;
	}
	#endif

	//Set VSYNC + Late Swap (prevents crazy FPS):
	if (SDL_GL_SetSwapInterval(-1) != 0) {
		std::cerr << "NOTE: couldn't set vsync + late swap tearing (" << SDL_GetError() << ")." << std::endl;
		if (SDL_GL_SetSwapInterval(1) != 0) {
			std::cerr << "NOTE: couldn't set vsync (" << SDL_GetError() << ")." << std::endl;
		}
	}

	//Hide mouse cursor (note: showing can be useful for debugging):
	SDL_ShowCursor(SDL_DISABLE);


	int num_sheep = DEFAULT_NUM_SHEEP;
	float r = 0.3f;

	float angle = 2.0f * M_PI / num_sheep;

	float sheep_speed = 0.1;
	int update = 0;

	int game_state = STOPPED;

	//------------  game state ------------

	Sheep* sheep = new Sheep[num_sheep];

	Animal dog;

	auto epoch = std::chrono::high_resolution_clock::now().time_since_epoch();
	auto val = std::chrono::duration_cast<std::chrono::milliseconds>(epoch);
	int seed = val.count();
	srand(seed);

	int dir, axis;

	for(int i = 0; i < num_sheep; i++) {
		sheep[i].pos.x = r * sin(i * angle);
		sheep[i].pos.y = r * cos(i * angle);
		sheep[i].dir.x = 0.0f;
		sheep[i].dir.y = 0.0f;
	}

	//------------  game loop ------------

	auto previous_time = std::chrono::high_resolution_clock::now();
	bool should_quit = false;
	int score = 0;
	auto game_start = std::chrono::high_resolution_clock::now();
	while (true) {
		static SDL_Event evt;
		while (SDL_PollEvent(&evt) == 1) {
			//handle input:
			if (evt.type == SDL_MOUSEMOTION) {
				dog.pos.x = (evt.motion.x + 0.5f) / float(config.size.x) * 2.0f - 1.0f;
				dog.pos.y = (evt.motion.y + 0.5f) / float(config.size.y) *-2.0f + 1.0f;
			} else if (evt.type == SDL_MOUSEBUTTONDOWN) {
				for (int i = 0; i < num_sheep; i++) {
					sheep[i].pos.x = r * sin(i * angle);
					sheep[i].pos.y = r * cos(i * angle);
					dir = (rand() % 2) * 2 - 1;
					axis = rand() % 2;
					sheep[i].dir.x = dir * axis;
					sheep[i].dir.y = dir * !axis;
				}
				sheep_speed = 0.1;
				update = 0;
				game_state = RUNNING;
				game_start = std::chrono::high_resolution_clock::now();
				score = 0;
			} else if (evt.type == SDL_KEYDOWN && evt.key.keysym.sym == SDLK_ESCAPE) {
				should_quit = true;
			} else if (evt.type == SDL_QUIT) {
				should_quit = true;
				break;
			}
		}
		if (should_quit) break;

		auto current_time = std::chrono::high_resolution_clock::now();
		float elapsed = std::chrono::duration< float >(current_time - previous_time).count();
		previous_time = current_time;

		if (game_state == RUNNING) { //update game state:
			bool hit_boundary;
			bool hit_sheep;
			int next_sheep;
			for (int i = 0; i < num_sheep; i++) {
				sheep[i].pos += elapsed * sheep_speed * sheep[i].dir;
				hit_boundary = 	sheep[i].pos.x < -0.85 ||
						sheep[i].pos.x >  0.85 ||
						sheep[i].pos.y < -0.85 ||
						sheep[i].pos.y >  0.85;

				if (hit_boundary) {
					game_state = STOPPED;
					auto game_end = std::chrono::high_resolution_clock::now();
					auto game_diff = std::chrono::duration_cast<std::chrono::seconds>(game_end - game_start);
					score = game_diff.count();
				}

				//check if another sheep has been hit
				for (int j = 1; j < num_sheep; j++) {
					next_sheep = (i + j) % num_sheep;
					hit_sheep = fabsf(sheep[next_sheep].pos.x - sheep[i].pos.x) <= 0.10f &&
						fabsf(sheep[next_sheep].pos.y - sheep[i].pos.y) <= 0.10f;
					if (hit_sheep && sheep[i].last_col != &sheep[next_sheep]) {
						sheep[i].last_col = &sheep[next_sheep];
						sheep[i].col_det = true;
						continue;
					} else if (!hit_sheep && sheep[i].last_col == &sheep[next_sheep]) {
						sheep[i].last_col = nullptr;
					}
				}

				//check if the dog has been hit
				float x = sheep[i].pos.x - dog.pos.x;
				float y = sheep[i].pos.y - dog.pos.y;
				if (fabsf(x) <= 0.10f && fabsf(y) <= 0.10f) {
					if (sheep[i].last_col != &dog) {
						sheep[i].col_det = true;
						sheep[i].last_col = &dog;
					} 
					if (fabsf(x) > fabsf(y)) {
						dog.pos.x = sheep[i].pos.x - ((x > 0.0f)?0.1f:-0.1f);
					} else {
						dog.pos.y = sheep[i].pos.y - ((y > 0.0f)?0.1f:-0.1f);
					}
				} else {
					if (sheep[i].last_col == &dog) {
						sheep[i].last_col = nullptr;
					}
				}
			}

			for (int i = 0; i < num_sheep; i++) {
				if (sheep[i].col_det == true) {
					float x = sheep[i].pos.x - sheep[i].last_col->pos.x;
					float y = sheep[i].pos.y - sheep[i].last_col->pos.y;
					if (fabsf(x) > fabsf(y)) {
						//change dir along x
						sheep[i].dir.y = 0.0f;
						sheep[i].dir.x = (x >= 0.0f) ? 1.0f : -1.0f;
					} else {
						//change dir along y
						sheep[i].dir.x = 0.0f;
						sheep[i].dir.y = (y >= 0.0f) ? 1.0f : -1.0f;
					}
					sheep[i].col_det = false;
				}
			}
		}

		//draw output:
		glClearColor(0.0f, 0.75f, 0.28f, 1.0f); //green color
		glClear(GL_COLOR_BUFFER_BIT);


		if (game_state == RUNNING) { //draw game state:
			Draw draw;
			draw.add_rectangle(glm::vec2(-0.95f,-0.95f), glm::vec2(0.95f, 0.95f), glm::u8vec4(0xbe, 0x75, 0x52, 0xff));
			draw.add_rectangle(glm::vec2(-0.90f,-0.90f), glm::vec2(0.90f, 0.90f), glm::u8vec4(0x00, 0xbe, 0x4c, 0xff));
			draw.add_rectangle(dog.pos + glm::vec2(-0.05f,-0.05f), dog.pos + glm::vec2(0.05f, 0.05f), glm::u8vec4(0x00, 0x00, 0x00, 0xff));
			for (int i = 0; i < num_sheep; i++) {
				draw.add_rectangle(sheep[i].pos + glm::vec2(-0.05f,-0.05f), sheep[i].pos + glm::vec2(0.05f, 0.05f), glm::u8vec4(0xff, 0xff, 0xff, 0xff));
			}
			draw.draw();
		} else {

			if (score != 0 ) {
				int num, temp;
				temp = score;
				float pos_diff = 0.0f;
				while (temp > 0) {
					num = temp % 10;
					draw_D7(pos_diff, 0.4f, arr[num][0], arr[num][1],
						arr[num][2], arr[num][3], arr[num][4], 
						arr[num][5], arr[num][6]);
					temp /= 10;
					pos_diff -= 0.08f;
				}
			}

			draw_D7(-0.50f, 0.2f, true, false, false, true, true, true, false);
			draw_D7(-0.42f, 0.2f, false, false, false, true, true, true, false);
			draw_D7(-0.34f, 0.2f, false, true, true, false, false, false, false);
			draw_D7(-0.26f, 0.2f, true, false, false, true, true, true, false);
			draw_D7(-0.18f, 0.2f, false, true, true, false, true, true, true);

			draw_D7(-0.02f, 0.2f, true, true, false, true, true, false, true);

			draw_D7(0.14f, 0.2f, true, false, true, true, false, true, true);
			draw_D7(0.22f, 0.2f, false, false, false, true, true, true, true);
			draw_D7(0.30f, 0.2f, true, true, true, false, true, true, true);
			draw_D7(0.38f, 0.2f, true, true, true, false, true, true, true);
			draw_D7(0.46f, 0.2f, false, false, false, true, true, true, true);
			
		}


		update++;
		if (update == 5) {
			update = 0;
			sheep_speed += 0.001f;
		}
		SDL_GL_SwapWindow(window);
	}


	//------------  teardown ------------

	SDL_GL_DeleteContext(context);
	context = 0;

	SDL_DestroyWindow(window);
	window = NULL;

	return 0;
}
