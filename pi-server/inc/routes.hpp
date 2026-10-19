#ifndef ROUTES_HPP
#define ROUTES_HPP

#include "httplib.h"
#include "Maze.hpp"
#include "MinitelController.hpp"

void registerRoutes(httplib::Server& srv, Maze& maze, MinitelController& controller);

#endif // ROUTES_HPP
