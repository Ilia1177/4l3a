#ifndef ROUTES_HPP
#define ROUTES_HPP

#include "Maze.hpp"
#include "MinitelController.hpp"
#include "httplib.h"

void registerRoutes(httplib::Server& srv, Maze& maze, MinitelController& controller);

#endif  // ROUTES_HPP
