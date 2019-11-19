/*******************************************************************
 ** This code is part of Breakout.
 **
 ** Breakout is free software: you can redistribute it and/or modify
 ** it under the terms of the CC BY 4.0 license as published by
 ** Creative Commons, either version 4 of the License, or (at your
 ** option) any later version.
 ******************************************************************/
#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <vector>
#include <string>
#include <tuple>
#include "box.h"
#include "buffer.h"

enum Direction { UP, DOWN, LEFT, RIGHT };

const glm::vec2 PLAYER_SIZE(100, 20);
const GLfloat PLAYER_VELOCITY(300.0f);

const glm::vec2 INITIAL_BALL_VELOCITY(100.0f, -350.0f);

const GLfloat BALL_RADIUS(12.5f);

enum GameState
  {
   GAME_ACTIVE,
   GAME_STOP,
   GAME_MENU,
   GAME_WIN
  };

struct Editor
{
public:
  // Game state
  GameState State;	
  GLboolean Keys[1024];
  GLuint    Width, Height;
  // std::string Content;
  Buffer    buffer;
  GLuint    cursorPos;

  // Constructor/Destructor
  Editor(GLuint width, GLuint height);
  ~Editor();
  
  void Init();

  void backspace();
  void move(int delta);
  void copy();
  void end_of_line (int num = 1);
  void beginning_of_line (int num = 1);
  int search (std::string regex, bool forward = true);

  void scroll(int offset = 1);

  void forward_line(int num = 1);

  void ProcessInput();
  void Update();
  void Render();

  void resize(int, int);
};
