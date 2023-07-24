/*
 *     .============.
 *    //  M A K E  / \
 *   //  C++ DEV  /   \
 *  //  E A S Y  /  \/ \
 * ++ ----------.  \/\  .
 *  \\     \     \ /\  /
 *   \\     \     \   /
 *    \\     \     \ /
 *     -============'
 *
 * Copyright (c) 2018 Hevake and contributors, all rights reserved.
 *
 * This file is part of cpp-tbox (https://github.com/cpp-main/cpp-tbox)
 * Use of this source code is governed by MIT license that can be found
 * in the LICENSE file in the root of the source tree. All contributing
 * project authors may be found in the CONTRIBUTORS.md file in the root
 * of the source tree.
 */
#include "game_lite.h"
#include <unistd.h>
#include <cmath>
#include <iostream>

using namespace tbox;
using namespace std;

GameLite::GameLite() :
    wp_loop_(nullptr),
    sp_30sec_timer_(nullptr),
    sp_20sec_timer_(nullptr),
    sp_10sec_timer_(nullptr),
    sp_stdin_read_ev_(nullptr),
    right_answer_(0),
    remain_question_number_(5)
{ }

GameLite::~GameLite()
{
}

void GameLite::init(Loop *wp_loop)
{
    wp_loop_ = wp_loop;

    sp_30sec_timer_ = wp_loop_->newTimerEvent();
    sp_20sec_timer_ = wp_loop_->newTimerEvent();
    sp_10sec_timer_ = wp_loop_->newTimerEvent();
    sp_stdin_read_ev_ = wp_loop_->newFdEvent();

    sp_30sec_timer_->initialize(std::chrono::seconds(30), Event::Mode::kOneshot);
    sp_30sec_timer_->setCallback(std::bind(&GameLite::on30SecReach, this));

    sp_20sec_timer_->initialize(std::chrono::seconds(20), Event::Mode::kOneshot);
    sp_20sec_timer_->setCallback(std::bind(&GameLite::on20SecReach, this));

    sp_10sec_timer_->initialize(std::chrono::seconds(10), Event::Mode::kOneshot);
    sp_10sec_timer_->setCallback(std::bind(&GameLite::on10SecReach, this));

    using std::placeholders::_1;
    sp_stdin_read_ev_->initialize(STDIN_FILENO, FdEvent::kReadEvent, Event::Mode::kPersist);
    sp_stdin_read_ev_->setCallback(std::bind(&GameLite::onStdinReadable, this, _1));

    cout << "Welcome to Calculate GameLite." << endl
         << "You need answer 5 questions in 30 second. Each question has only 10 second." << endl
         << "Here we go!" << endl;

    srand(time(NULL));
    sp_stdin_read_ev_->enable();
    sp_30sec_timer_->enable();
    sp_20sec_timer_->enable();

    askQuestion();
}

void GameLite::cleanup()
{
    delete sp_30sec_timer_;
    delete sp_20sec_timer_;
    delete sp_10sec_timer_;
    delete sp_stdin_read_ev_;
}

void GameLite::askQuestion()
{
    sp_10sec_timer_->disable();

    int a = random() % 100;
    int b = random() % 100;
    right_answer_ = a + b;
    cout << a << " + " << b << " = ? " << endl << "your answer: " << flush;

    --remain_question_number_;
    sp_10sec_timer_->enable();
}

void GameLite::on30SecReach()
{
    cout << endl << "time is up, you fail!" << endl;
    wp_loop_->exitLoop();
}

void GameLite::on20SecReach()
{
    cout << endl << "10 sec remain ..." << endl;
}

void GameLite::on10SecReach()
{
    cout << endl << "timeout. You fail!" << endl;
    wp_loop_->exitLoop();
}

void GameLite::onStdinReadable(short event)
{
    char input_buff[200];
    int rsize = read(STDIN_FILENO, input_buff, sizeof(input_buff));
    input_buff[rsize - 1] = '\0';

    int answer = atoi(input_buff);
    if (answer == right_answer_) {
        if (remain_question_number_ == 0) {
            cout << "Congratulation! You win." << endl;

            wp_loop_->exitLoop();

        } else {
            askQuestion();
        }
    } else {
        cout << "wrong answer. You fail!" << endl;
        wp_loop_->exitLoop();
    }
}
