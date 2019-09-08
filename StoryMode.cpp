#include "StoryMode.hpp"

#include "Sprite.hpp"
#include "DrawSprites.hpp"
#include "Load.hpp"
#include "data_path.hpp"
#include "gl_errors.hpp"
#include "MenuMode.hpp"

Sprite const *sprite_red_flag = nullptr;
Sprite const *sprite_red_car = nullptr;
Sprite const *sprite_yellow_car = nullptr;

const std::vector<glm::vec2> redcar_pos = {{205, 183}, {150, 183}, {95, 183}, {40, 183}};
const std::vector<glm::vec2> yellowcar_pos = {{205, 136}, {150, 136}, {95, 136}, {40, 136}};

const std::vector<std::string> questions = {
    "What's the motto of CMU ?",
    "Which year was CMU founded?",
    "Which shuttle goes to Squirrel Hill?",
    "How many schools/colleges in CMU?",
    "How many games will we make?",
};

const std::vector<std::vector<std::string>> choices = {
    {
        "A. My heart is in the movie",
        "B. My heart is in the game",
        "C. My heart is in the work"
    },
    {
        "A. 1890",
        "B. 1900",
        "C. 1910"
    },
    {
        "A. A Route",
        "B. B Route",
        "C. PTC Route"
    },
    {
        "A. 5",
        "B. 6",
        "C. 7"
    },
    {
        "A. 7",
        "B. 8",
        "C. 9"
    }
};

const std::vector<unsigned int> answers = {
        2,
        1,
        1,
        2,
        1
};

Load< SpriteAtlas > sprites(LoadTagDefault, []() -> SpriteAtlas const * {
	SpriteAtlas const *ret = new SpriteAtlas(data_path("car-race"));

	sprite_red_flag = &ret->lookup("red-flag");
    sprite_red_car = &ret->lookup("red-car");
    sprite_yellow_car = &ret->lookup("yellow-car");

	return ret;
});

StoryMode::StoryMode() {
    redcar_x = redcar_pos[sizeof(redcar_pos) - 1].x;
    yellowcar_x = yellowcar_pos[sizeof(yellowcar_pos) - 1].x;
}

StoryMode::~StoryMode() {
}

bool StoryMode::handle_event(SDL_Event const &evt, glm::uvec2 const &window_size) {
    if (evt.type == SDL_KEYDOWN && win) {
        Mode::current = nullptr;
        return true;
    }
	if (Mode::current.get() != this) return false;

	return false;
}

void StoryMode::update(float elapsed) {
    redcar_x = std::max(redcar_x - 30.f * elapsed, redcar_pos[redcar_index].x);
    yellowcar_x = std::max(yellowcar_x - 30.f * elapsed, yellowcar_pos[yellowcar_index].x);
	if (Mode::current.get() == this) {
		//there is no menu displayed! Make one:
		enter_scene();
	}
}

void StoryMode::enter_scene() {
	//just entered this scene, adjust flags and build menu as appropriate:
	std::vector< MenuMode::Item > items;
	glm::vec2 at(3.0f, view_max.y - 123.0f);
	auto add_text = [&items,&at](const std::string& words) {
		items.emplace_back(words, nullptr, 1.0f, nullptr, at);
		at.y -= 15.0f;
	};
	auto add_choice = [&items,&at](const std::string& words, std::function< void(MenuMode::Item const &) > const &fn) {
		items.emplace_back(words, nullptr, 1.0f, fn, at + glm::vec2(8.0f, 0.0f));
		at.y -= 20.0f;
	};

	add_text(questions[question_index]);
    at.y -= 8.0f; //gap before choices
    for (unsigned int i = 0; i < choices[question_index].size(); ++i) {
        add_choice(choices[question_index][i], [this, i](MenuMode::Item const &){
            if (i == answers[question_index]) {
                yellowcar_index++;
            } else {
                redcar_index++;
            }
            question_index++;
            if (yellowcar_index + 1 == yellowcar_pos.size() || redcar_index + 1 == redcar_pos.size()) {
                win = true;
                question_index--;
            }
            Mode::current = shared_from_this();
        });
    }

	std::shared_ptr< MenuMode > menu = std::make_shared< MenuMode >(items);
	menu->atlas = sprites;
	menu->left_select = nullptr;
	menu->right_select = nullptr;
	menu->view_min = view_min;
	menu->view_max = view_max;
	menu->background = shared_from_this();
	Mode::current = menu;
}

void StoryMode::draw(glm::uvec2 const &drawable_size) {
	//clear the color buffer:
	glClearColor(1.0f, 1.0f, 0.875f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	//use alpha blending:
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	//don't use the depth test:
	glDisable(GL_DEPTH_TEST);

	{ //use a DrawSprites to do the drawing:
		DrawSprites draw(*sprites, view_min, view_max, drawable_size, DrawSprites::AlignPixelPerfect);
		glm::vec2 ul = glm::vec2(view_min.x, view_min.y);

		draw.draw(*sprite_red_flag, ul + glm::vec2(20, 130), 0.3);
		draw.draw(*sprite_red_car, ul + glm::vec2(redcar_x, redcar_pos[redcar_index].y), 0.07);
		draw.draw(*sprite_yellow_car, ul + glm::vec2(yellowcar_x, yellowcar_pos[yellowcar_index].y), 0.07);

		if (win) {
            if (redcar_index > yellowcar_index) {
                draw.draw_text("Winner!", ul + glm::vec2(-20, 188), 1, glm::u8vec4(0xff, 0x00, 0x00, 0xff));
            } else {
                draw.draw_text("Winner!", ul + glm::vec2(-20, 145), 1, glm::u8vec4(0xff, 0x00, 0x00, 0xff));
            }
		} else {
            draw.draw_text("You", ul + glm::vec2(-10, 145), 1, glm::u8vec4(0x00, 0x00, 0xff, 0xff));
        }
	}
	GL_ERRORS(); //did the DrawSprites do something wrong?
}
