#include "enemies.hpp"

#include <cppitertools/itertools.hpp>
#include <glm/gtx/fast_trigonometry.hpp>


void Enemies::initializeGL(GLuint program) {
  terminateGL();
  // Start pseudo-random number generator
  auto seed{std::chrono::steady_clock::now().time_since_epoch().count()};
  m_randomEngine.seed(seed);

  m_program = program;
  m_colorLoc = glGetUniformLocation(m_program, "color");
  m_rotationLoc = glGetUniformLocation(m_program, "rotation");
  m_scaleLoc = glGetUniformLocation(m_program, "scale");
  m_translationLoc = glGetUniformLocation(m_program, "translation");
  // Aumenta difculdade a cada restart game
  
  // instanciando inimigos
  m_enemies.clear();
  m_enemies.resize(CONST_QUANTIDADE_NAVES);


   // -1 < x < 1
   // -0.8     -0.4    -0     0.4    0.8
   //     -0.6     -0.2   0.2    0.6     
   // -0.8     -0.4    -0     0.4    0.8

    double x = -0.8;
    int index = 1;
    for (auto &enemy : m_enemies) {
    enemy = createEnemy();

    if(1 <= index && index < 6){
      enemy.m_translation = {x, 0.9};
      x += 0.4; 
    }
    else if(6 <= index && index < 10 ){

      if(index == 6)
        x = -0.6;

      enemy.m_translation = {x, 0.7};
      x += 0.4; 
    }
    else if(10 <= index && index <= CONST_QUANTIDADE_NAVES){

      if(index == 10)
        x = -0.8;

      enemy.m_translation = {x, 0.5};
      x += 0.4; 
    }
    index++;
  }
}
   


void Enemies::paintGL() {
  glUseProgram(m_program);

  for (auto &enemy : m_enemies) {
    glBindVertexArray(enemy.m_vao);

    glUniform4fv(m_colorLoc, 1, &enemy.m_color.r);
    glUniform1f(m_scaleLoc, enemy.m_scale);

    for (auto i : {-2, 0, 2}) {
      for (auto j : {-2, 0, 2}) {
        glUniform2f(m_translationLoc, enemy.m_translation.x + j,
                    enemy.m_translation.y + i);

        glDrawArrays(GL_TRIANGLE_FAN, 0, enemy.m_polygonSides + 2);
      }
    }

    glBindVertexArray(0);
  }

  glUseProgram(0);
}

void Enemies::terminateGL() {
  for (auto enemy : m_enemies) {
    glDeleteBuffers(1, &enemy.m_vbo);
    glDeleteVertexArrays(1, &enemy.m_vao);
  }
}

void Enemies::update(GameData m_gameData, float deltaTime) {

  tempo_atual_restante -= deltaTime;

  if(tempo_atual_restante <= 0)
  {
    sentido *= (-1);
    tempo_atual_restante = CONST_TEMPO_ZIG_ZAG;
  }

  for (auto &enemy : m_enemies) {
    enemy.m_translation.y -= m_gameData.fator_vel_jogo * deltaTime;
    enemy.m_translation.x -= sentido * deltaTime;
    enemy.m_rotation = glm::wrapAngle(
        enemy.m_rotation + enemy.m_angularVelocity * deltaTime);
  
    // Wrap-around
    if (enemy.m_translation.x < -1.0f) enemy.m_translation.x += 2.0f;
    if (enemy.m_translation.x > +1.0f) enemy.m_translation.x -= 2.0f;
    if (enemy.m_translation.y < -1.0f) enemy.m_translation.y += 2.0f;
    if (enemy.m_translation.y > +1.0f) enemy.m_translation.y -= 2.0f;
  }
}

Enemies::Enemy Enemies::createEnemy(glm::vec2 translation) {
  Enemy enemy;

  auto &re{m_randomEngine};  // Shortcut

  // Randomly choose the number of sides
  enemy.m_polygonSides = 16;

  // Choose a random color (actually, a grayscale)
  std::uniform_real_distribution<float> randomIntensity(0.5f, 1.0f);
  enemy.m_color = glm::vec4 {1.0f,0.5f,0.5f, 1.0f};

  enemy.m_color.a = 1.0f;
  enemy.m_rotation = 0.0f;
  enemy.m_scale = 0.02;
  enemy.m_translation = translation;

  // Choose a random angular velocity
  enemy.m_angularVelocity = m_randomDist(re);

  // Choose a random direction
  glm::vec2 direction{m_randomDist(re), m_randomDist(re)};
  enemy.m_velocity = glm::normalize(direction) / 7.0f;

  // Create geometry
  // std::vector<glm::vec2> positions(0);
  std::array<glm::vec2, 16> positions{

      // Ship body
      //N?? RAIZ para ele poder renderizar corretamente o GL_TRIANGLE_FAN
      glm::vec2{0.0f, 0.0f},

      glm::vec2{+00.5f, -02.0f}, glm::vec2{+00.5f, -05.0f}, //1,2
      glm::vec2{+03.0f, -00.5f}, glm::vec2{+03.0f, +01.5f}, //3,4
      glm::vec2{+01.5f, +02.5f}, glm::vec2{+00.5f, +02.5f}, //5,6
      glm::vec2{+00.5f, +02.0f}, glm::vec2{-00.5f, +02.0f},  //7,8
      glm::vec2{-00.5f, +02.5f}, glm::vec2{-01.5f, +02.5f},  //9,10
      glm::vec2{-03.0f, +01.5f}, glm::vec2{-03.0f, -00.5f}, //11,12
      glm::vec2{-00.5f, -05.0f}, glm::vec2{-00.5f, -02.0f}, //13,14

      //O primeiro n?? novamente para ele fechar o leque
      glm::vec2{+00.5f, -02.0f},
      };

  // Generate VBO
  glGenBuffers(1, &enemy.m_vbo);
  glBindBuffer(GL_ARRAY_BUFFER, enemy.m_vbo);
  glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(glm::vec2),
               positions.data(), GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  // Get location of attributes in the program
  GLint positionAttribute{glGetAttribLocation(m_program, "inPosition")};

  // Create VAO
  glGenVertexArrays(1, &enemy.m_vao);

  // Bind vertex attributes to current VAO
  glBindVertexArray(enemy.m_vao);

  glBindBuffer(GL_ARRAY_BUFFER, enemy.m_vbo);
  glEnableVertexAttribArray(positionAttribute);
  glVertexAttribPointer(positionAttribute, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  // End of binding to current VAO
  glBindVertexArray(0);

  return enemy;
}
