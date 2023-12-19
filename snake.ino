#include <UTFT.h>

// Дефайны джостика
#define pinX        A2
#define pinY        A1
#define swPin       2
#define ERR_INP_X   0
#define ERR_INP_Y   -8
// Дефайны экрана
#define dispMISO    8
#define dispSCK     7
#define dispCS      6
#define dispRST     5
#define dispDC      4
// Дефайны цветов
#define foodColor   227, 41, 41
#define tailColor   14, 74, 23
#define headColor   30, 230, 70
#define emptyColor  0, 0, 0
// Дефайны поля
#define playFieldX  16
#define playFieldY  12
#define SPEED       0.5
// Инициализация дисплея
UTFT myGLCD(TFT01_24SP, dispMISO,
 dispSCK, dispCS,
 dispRST, dispDC);
// Структура позиции (по факту просто пара X;Y)
struct location {
  int8_t x;
  int8_t y;
};


// Метод отрисовки квадратиков
void draw(location pos, int8_t r, int8_t g, int8_t b) {
  myGLCD.setColor(r,g,b);
  myGLCD.fillRect(pos.x*20, pos.y*20, (pos.x+1)*20, (pos.y+1)*20);
};

// Определяем глобальные переменные игры
location tail[playFieldX * playFieldY + 1];
location head;
bool isOver;
location facing;
location food;
int score = 0;
location temp;
bool lastTickFood = false;


// Сброс Игры
void gameSetup() {
  myGLCD.clrScr();
  head = {playFieldY / 2,playFieldX / 2};
  isOver = false;
  facing = {1,0};
  newFood();
  score = 0;
}

// Общие Настройки ардуины
void setup() {
  Serial.begin(9600);
  randomSeed(analogRead(A4));
  myGLCD.InitLCD();

  pinMode(pinX, INPUT);
  pinMode(pinY, INPUT);

  pinMode(swPin, INPUT);
  digitalWrite(swPin, HIGH);

  gameSetup();
}

// Читаем ввод джостика, преобразуем в переменную facing
void input() {
  // Считываем данные и убираем погрешность
  int X = analogRead(pinX);
  int Y = analogRead(pinY);
  X += ERR_INP_X;
  Y += ERR_INP_Y;
  // Считаем модули по значениямджостикам
  int dY = Y - 512;
  int dX = X - 512;
  if (dY < 0) {
    dY *= -1;
  }
  if (dX < 0) {
    dX *= -1;
  }
  if (X > 550 && dX >= dY) {
    facing.x = 0;
    facing.y = -1;
  } else if (X < 450 && dX >= dY) {
    facing.x = 0;
    facing.y = 1;
  } else if (Y > 550 && dY >= dX) {
    facing.x = 1;
    facing.y = 0;
  } else if (Y < 450 && dY >= dX) {
    facing.x = -1;
    facing.y = 0;
  }
}
// Проверка на работоспособность игры
bool checkGameEnd() {
  if (head.x >= playFieldX || head.y >= playFieldY || head.x < 0 || head.y < 0) {
    return true;
  }
  for (int i = 0; i < score; ++i) {
    if (head.x == tail[i].x && head.y == tail[i].y) {
      return true;
    }
  }
  return false;
}
// Такт Игры
void move() {
  location oldHead = {head.x, head.y};
  head.x += facing.x;
  head.y += facing.y;
  if (checkGameEnd()) {
    isOver = true;
    return;
  }
  draw({head.x, head.y}, headColor);
  if (lastTickFood) {
    lastTickFood = false;
    draw({oldHead.x, oldHead.y}, tailColor);
    tail[score] = {oldHead.x, oldHead.y};
    score++;
    newFood();
  } else {
    if (score > 0) {
      draw(tail[0], emptyColor);
      tail[score].x = oldHead.x;
      tail[score].y = oldHead.y;
      draw(oldHead, tailColor);
      for (int i = 0; i < score; ++i) {
        tail[i].x = tail[i + 1].x;
        tail[i].y = tail[i + 1].y;
      }
    } else {
      if (facing.x != 0 || facing.y != 0) {
        draw(oldHead, emptyColor);
      }
    }
  }
  if (food.x == head.x && food.y == head.y) {
    lastTickFood = true;
  }
}

// Спавн Новой Еды
void newFood() {
  while (true) {
    location spawn = {random(0, playFieldX - 1), random(0, playFieldY - 1)};
    if (spawn.x == head.x && spawn.y == head.y) {
      continue;
    }
    bool validSpawn = true;
    for (int i = 0; i < score; ++i) {
      if (tail[i].x == spawn.x && tail[i].y == spawn.y) {
        validSpawn = false;
        break;
      }
    }
    if (!validSpawn) {
      continue;
    }
    food.x = spawn.x;
    food.y = spawn.y;
    draw({spawn.x, spawn.y}, foodColor);
    break;
  }
}


// Цикл Игры
void loop() {
  while (!isOver) {
    for (int i = 0; i < 10; ++i) {
      input();
      delay(floor(25.0 / SPEED));
      
    }
    move();
  }
  gameSetup();
}
