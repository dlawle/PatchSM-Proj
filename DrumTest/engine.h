#pragma once
#ifndef ENGINE_H
#define ENGINE_H

#include "daisy.h"
#include "daisysp.h"

using namespace daisy;
using namespace daisysp;
using namespace patch_sm;

extern daisy::patch_sm::DaisyPatchSM hw;

const static int euc16[17][16] = {//euclidian rythm
 {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
 {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
 {1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0},
 {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0},
 {1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0},
 {1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0},
 {1, 0, 0, 1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1, 0, 0},
 {1, 0, 0, 1, 0, 1, 0, 1, 0, 0, 1, 0, 1, 0, 1, 0},
 {1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0},
 {1, 0, 1, 1, 0, 1, 0, 1, 0, 1, 1, 0, 1, 0, 1, 0},
 {1, 0, 1, 1, 0, 1, 0, 1, 1, 0, 1, 1, 0, 1, 0, 1},
 {1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1},
 {1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1},
 {1, 0, 1, 1, 1, 1, 0, 1, 1, 1, 1, 0, 1, 1, 1, 1},
 {1, 0, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1},
 {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0},
 {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}
};


class engine
{
private:
    int i = 0;

    //push button
    bool sw = 0;//push button
    bool old_sw;//countermeasure of sw chattering
    unsigned long sw_timer = 0;//countermeasure of sw chattering

    //each channel param
    int hits[6] = { 4, 4, 5, 3, 2, 16};//each channel hits
    int offset[6] = { 0, 2, 0, 8, 3, 9};//each channele step offset
    bool mute[6] = {0, 0, 0, 0, 0, 0}; //mute 0 = off , 1 = on
    int limit[6] = {16, 16, 16, 16, 16, 16};//eache channel max step


    //Sequence variable
    int j = 0;
    int k = 0;
    int m = 0;
    int buf_count = 0;

    // bools for channel triggers
    bool trig1, trig2, trig3, trig4;

    bool offset_buf[6][16];//offset buffer , Stores the offset result

    bool trg_in = 0;//external trigger in H=1,L=0
    bool old_trg_in = 0;
    int playing_step[6] = {0, 0, 0, 0, 0, 0}; //playing step number , CH1,2,3,4,5,6
    unsigned long gate_timer = 0;//countermeasure of sw chattering

    // random creator 
    int randomGen(int min, int max)
    {
        int randNum = rand()%(max-min + 1) + min;
        return randNum;
    }

    //random assign
    int hit_occ[6] = {0, 10, 20, 20, 40, 80}; //random change rate of occurrence
    int off_occ[6] = {10, 20, 20, 30, 40, 20}; //random change rate of occurrence
    int mute_occ[6] = {20, 20, 20, 20, 20, 20}; //random change rate of occurrence
    int hit_rng_max[6] = {0, 14, 16, 8, 9, 16}; //random change range of max
    int hit_rng_min[6] = {0, 13, 6, 1, 5, 10}; //random change range of max

    int bar_now = 1;//count 16 steps, the bar will increase by 1.
    int bar_max[4] = {2, 4, 8, 16} ;//selectable bar
    int bar_select = 1;//selected bar
    int step_cnt = 0;//count 16 steps, the bar will increase by 1.

    // mode selection (only useful for changing params)
    int mod = 0;

public:

int getMod()
{
    return mod;
}

void UpdateParams(int channel, float p1, float p2, float p3)
{
    hits[channel]   = p1;
    offset[channel] = p2;
    limit[channel]  = p3;
}

void randSetup()
{
     for (size_t i = 0; i < 5; i++)
        {
            hits[i] = randomGen(0, 16);
            offset[i] = randomGen(0, 16);
            limit[i] = randomGen(0, 16);
        }
}

bool GetTrig1()
{
    return trig1;
}

bool GetTrig2()
{
    return trig2;
}

bool GetTrig3()
{
    return trig3;
}

bool GetTrig4()
{
    return trig4;
}

bool Trig[4]
{
  GetTrig1(),
  GetTrig2(),
  GetTrig3(),
  GetTrig4()
};

 //-----------------push button----------------------
void run(){

 //-----------------offset setting----------------------
 for (k = 0; k <= 5; k++) { //k = 1~6ch
   for (i = offset[k]; i <= 15; i++) {
     offset_buf[k][i - offset[k]] = euc16[hits[k]][i];
   }

   for (i = 0; i < offset[k]; i++) {
     offset_buf[k][16 - offset[k] + i] = euc16[hits[k]][i];
   }
 }

 //-----------------trigger detect & output----------------------
 bool trg_in = hw.gate_in_1.Trig();//external trigger in
 if(old_trg_in == 0 && trg_in) {
   gate_timer = daisy::System::GetNow();
   for (i = 0; i <= 5; i++) {
     playing_step[i]++;      //When the trigger in, increment the step by 1.
     if (playing_step[i] >= limit[i]) {
       playing_step[i] = 0;  //When the step limit is reached, the step is set back to 0.
     }
   }
   for (k = 0; k <= 5; k++) {//output gate signal
     if (offset_buf[k][playing_step[k]] == 1 && mute[k] == 0) {
       switch (k) {
         case 0://CH1
           gen_led1.Write(true);
           trig1 = true;
           break;

         case 1://CH2
           gen_led2.Write(true);
           trig2 = true;
           break;

         case 2://CH3
           cv_led1.Write(true);
           trig3 = true;
           break;

         case 3://CH4
           cv_led2.Write(true);
           trig4 = true;
           break;

       }
     }
   }
 }

 if(gate_timer + 10 <= daisy::System::GetNow()) { //off all gate , gate time is 10msec
    gen_led1.Write(false);
    gen_led2.Write(false);
    cv_led1.Write(false);
    cv_led2.Write(false);
    trig1 = false;
    trig2 = false;
    trig3 = false;
    trig4 = false;
 }

}

void Random_change() { // when random mode and full of bar_now ,
 for (k = 1; k <= 5; k++) {

   if (hit_occ[k] >= randomGen(1, 100)) { //hit random change
     hits[k] = randomGen(hit_rng_min[k], hit_rng_max[k]);
   }

   if (off_occ[k] >= randomGen(1, 100)) { //hit random change
     offset[k] = randomGen(0, 16);
   }

   if (mute_occ[k] >= randomGen(1, 100)) { //hit random change
     mute[k] = 1;
   }
   else if (mute_occ[k] < randomGen(1, 100)) { //hit random change
     mute[k] = 0;
   }
 }
}

};
#endif //ENGINE_H