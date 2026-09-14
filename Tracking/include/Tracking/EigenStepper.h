/**
 * @file EigenStepper.h
 * The only reason this file exists is to silence an annoying
 * maybe-uninitialized warning that originates from the EigenStepper
 * in Acts.
 */
#pragma once
#ifndef TRACKING_EIGENSTEPPER_H
#define TRACKING_EIGENSTEPPER_H

#if !defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
#endif
#include "Acts/Propagator/EigenStepper.hpp"
#if !defined(__clang__)
#pragma GCC diagnostic pop
#endif

#endif
