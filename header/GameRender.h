#pragma once

#include "Graphics.h"
#include "Rendr.h"
#include "AnimationManager.h"

#include "Note.h"
#include "Player.h"
#include "Generator.h"
#include <vector>
#include <iostream>
#include <string>
#include <map>

class GameRender : public Rendr{
public:
    GameRender();
	void init(GLFWwindow* w);
	void highway(double time);
    void clicker();
    void notes(double time,std::vector<Note> &v, std::vector<Note>& cross);
    void events(double time, std::vector<Note>&ev, std::vector<Note>& cross);
    void lanes(double time, std::vector<Note> &v,std::vector<Note>&ev);
	void bpmTicks(double time, std::vector<double>& bpm_arr);
	void clickerAnimation();
	void result(Player& player,Generator& generator);
	void meters();
    void pollState(double time,Player& p,Generator &g);
	void updateAnimations(double time);
	void debug(std::vector<Note>& note, std::vector<Note>& ev, std::vector<Note>& c);
	void reset();
    virtual ~GameRender();

	bool m_isButtonsRight = false;
	float m_noteVisibleTime = 1.0;
private:
	std::vector<Note> getCrossInsideNote(Note& note, std::vector<Note> crossArr);

	bool m_red = false, m_green = false, m_blue = false;
    int m_playerCross = 1;
	int m_playerCombo = 0;
	int m_playerMult = 0;
	int m_playerScore = 0;
	int m_renderCross = 1;
	double m_renderEuValue = 0.0;
	bool m_renderEuActive = false;
	bool m_renderEuZone = false;

	glm::mat4 m_persProj = glm::mat4(1.0f);
	glm::mat4 m_orthoProj = glm::mat4(1.0f);

	unsigned int m_textureProgram = 0;
	unsigned int m_colorProgram = 0;
	unsigned int m_textProgram = 0;
	unsigned int m_highwayTexture = 0;
	unsigned int m_objTexture = 0;
	unsigned int m_metersTexture = 0;
	unsigned int m_clickerAnimation = 0;
	unsigned int m_fontTexture = 0;
	unsigned int m_textureVAO = 0;
	unsigned int m_textureVBO = 0;
	unsigned int m_colorVAO = 0;
	unsigned int m_colorVBO = 0;


	FT_Library m_FTLibrary;
	FT_Face m_font;
	
	AnimationManager m_animManager;

	//default green clicker position
	float m_greenLeft = 0.0f;
	float m_greenRight = 0.0f;
	float m_greenBack = 0.0f;
	float m_greenFront = 0.0f;

	//default blue clicker position
	float m_blueLeft = 0.0f;
	float m_blueRight = 0.0f;
	float m_blueBack = 0.0f;
	float m_blueFront = 0.0f;

	
};

