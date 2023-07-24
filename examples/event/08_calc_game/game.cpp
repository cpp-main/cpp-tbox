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
#include "game.h"
#include <unistd.h>
#include <cmath>
#include <iostream>

using namespace tbox;
using namespace std;

Game::Game() :
    wp_loop_(nullptr),
    sp_start_timer_(nullptr),
    sp_30sec_timer_(nullptr),
    sp_20sec_timer_(nullptr),
    sp_10sec_timer_(nullptr),
    sp_stdin_read_ev_(nullptr),
    start_countdown_(3),
    right_answer_(0),
    remain_question_number_(5),
    start_tstamp_(0),
    user_start_(false)
{ }

Game::~Game()
{ }

void Game::init(Loop *wp_loop)
{
    wp_loop_ = wp_loop;

    sp_start_timer_ = wp_loop_->newTimerEvent();
    sp_30sec_timer_ = wp_loop_->newTimerEvent();
    sp_20sec_timer_ = wp_loop_->newTimerEvent();
    sp_10sec_timer_ = wp_loop_->newTimerEvent();
    sp_stdin_read_ev_ = wp_loop_->newFdEvent();

    sp_start_timer_->initialize(std::chrono::seconds(1), Event::Mode::kPersist);
    sp_start_timer_->setCallback(std::bind(&Game::onStartGame, this));

    sp_30sec_timer_->initialize(std::chrono::seconds(30), Event::Mode::kOneshot);
    sp_30sec_timer_->setCallback(std::bind(&Game::on30SecReach, this));

    sp_20sec_timer_->initialize(std::chrono::seconds(20), Event::Mode::kOneshot);
    sp_20sec_timer_->setCallback(std::bind(&Game::on20SecReach, this));

    sp_10sec_timer_->initialize(std::chrono::seconds(10), Event::Mode::kOneshot);
    sp_10sec_timer_->setCallback(std::bind(&Game::on10SecReach, this));

    using std::placeholders::_1;
    sp_stdin_read_ev_->initialize(STDIN_FILENO, FdEvent::kReadEvent, Event::Mode::kPersist);
    sp_stdin_read_ev_->setCallback(std::bind(&Game::onStdinReadable, this, _1));

    cout << "Welcome to Calculate Game." << endl
         << "You need answer 5 questions in 30 second. Each question has only 10 second." << endl
         << "Press ENTER to start." << endl;

    sp_stdin_read_ev_->enable();
}

void Game::cleanup()
{
    delete sp_stdin_read_ev_;
    delete sp_10sec_timer_;
    delete sp_20sec_timer_;
    delete sp_30sec_timer_;
    delete sp_start_timer_;
}

void Game::onStartGame()
{
    if (start_countdown_ > 0) {
        cout << start_countdown_ << endl;
        --start_countdown_;
        return;
    }

    cout << "Go!" << endl << endl;

    start_tstamp_ = time(NULL);
    srand(start_tstamp_);

    sp_start_timer_->disable();
    askQuestion();
    sp_30sec_timer_->enable();
    sp_20sec_timer_->enable();
}

void Game::askQuestion()
{
    sp_10sec_timer_->disable();

    int a = random() % 100;
    int b = random() % 100;
    right_answer_ = a + b;
    cout << a << " + " << b << " = ? " << endl << "your answer: " << flush;

    --remain_question_number_;
    sp_10sec_timer_->enable();
}

void Game::on30SecReach()
{
    cout << endl << "time is up, you fail!" << endl;
    wp_loop_->exitLoop();
}

void Game::on20SecReach()
{
    cout << endl << "10 sec remain ..." << endl;
}

void Game::on10SecReach()
{
    cout << endl << "timeout. You fail!" << endl;
    wp_loop_->exitLoop();
}

void Game::onStdinReadable(short event)
{
    char input_buff[200];
    int rsize = read(STDIN_FILENO, input_buff, sizeof(input_buff));
    input_buff[rsize - 1] = '\0';

    if (!user_start_) {
        cout << "Here we go!" << endl;
        sp_start_timer_->enable();
        user_start_ = true;
        return;
    }

    int answer = atoi(input_buff);
    if (answer == right_answer_) {
        if (remain_question_number_ == 0) {
            cout << "Congratulation! You win." << endl;
            time_t now = time(NULL);
            cout << "You cast " << now - start_tstamp_ << " sec." << endl;

            wp_loop_->exitLoop();

        } else {
            askQuestion();
        }
    } else {
        cout << "wrong answer. You fail!" << endl;
        wp_loop_->exitLoop();
    }
}
