#pragma once

enum Buttons
{
  up_button,
  down_button
};

extern volatile bool longPresses[2];
extern volatile bool shortPresses[2];

void resetButtonStates();
void enableButtonInterrupts();
void initButtonHandler();

