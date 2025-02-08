#version 450 core

#include "common.incl"

bool Interval_Contains(in Interval self, in float x) {
    return self.min < x && x < self.max;
}

bool Interval_Surrounds(in Interval self, in float x) {
    return self.min <= x && x <= self.max;
}
