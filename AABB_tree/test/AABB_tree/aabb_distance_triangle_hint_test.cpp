// Copyright (c) 2009 INRIA Sophia-Antipolis (France).
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org); you may redistribute it under
// the terms of the Q Public License version 1.0.
// See the file LICENSE.QPL distributed with CGAL.
//
// Licensees holding a valid commercial license may use this file in
// accordance with the commercial license agreement provided with the software.
//
// This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
// WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
//
// $URL: $
// $Id: $
//
//
// Author(s)     : Camille Wormser, Pierre Alliez
//
//******************************************************************************
// File Description :
//
//******************************************************************************

#include <iostream>
#include <fstream>
#include <CGAL/Timer.h>

#include <CGAL/AABB_intersections.h>
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Cartesian.h>
#include <CGAL/Simple_cartesian.h>

#include <CGAL/AABB_tree.h>
#include <CGAL/AABB_traits.h>
#include <CGAL/Polyhedron_3.h>
#include <CGAL/IO/Polyhedron_iostream.h>
#include <CGAL/AABB_polyhedron_triangle_primitive.h>

#include <CGAL/spatial_sort.h>

#include "AABB_test_util.h"

#define NBQ 100000

template<class Value>
size_t check_outputs(const std::vector<Value>& a, const std::vector<Value>& b, Value deflt) {
        size_t counter = 0;
        for(size_t i = 0; i < a.size(); ++i) {
                if(a[i] != b[i] && b[i] != deflt)
                        ++counter;
        }
        return counter;
}

template <class Tree, class K>
void test_hint_strategies(Tree& tree, CGAL::Polyhedron_3<K>& polyhedron)
{
        typedef typename K::Point_3 Point;
        typedef typename Tree::Primitive::Id Id;
        typedef typename Tree::Point_and_primitive_id Point_and_primitive_id;

        std::vector<Point> queries;
        std::vector<Id> outputs1, outputs2, outputs3;

        queries.reserve(NBQ);
        outputs1.reserve(NBQ);
        outputs2.reserve(NBQ);
        outputs3.reserve(NBQ);

//        size_t common_min = NBQ;
        size_t counter;

        for(size_t i = 0; i < NBQ; ++i)
                queries.push_back(random_point_in<K>(tree.bbox()));

        CGAL::spatial_sort(queries.begin(), queries.end());

        CGAL::Timer timer;
        timer.start();
        counter = 0;
        while(timer.time() < 1. && counter < NBQ) {
                outputs1.push_back(tree.closest_point_and_primitive(queries[counter]).second);
                ++counter;
        }
        timer.stop();
        double speed = static_cast<double>(counter)/(counter == NBQ? timer.time(): 1.);
        std::cout << "without hint:      " << speed << " queries/s" << std::endl;
        timer.reset();

        Point_and_primitive_id hint = tree.any_reference_point_and_id();

        timer.start();
        counter = 0;
        while(timer.time() < 1. && counter < NBQ) {
                outputs2.push_back((hint = tree.closest_point_and_primitive(queries[counter], hint)).second);
                ++counter;
        }
        timer.stop();
        speed = static_cast<double>(counter)/(counter == NBQ? timer.time(): 1.);
        std::cout << "with spatial sort: " << speed << " queries/s" << std::endl;
        timer.reset();

        tree.accelerate_distance_queries(polyhedron.points_begin(),polyhedron.points_end());

        timer.start();
        counter = 0;
        while(timer.time() < 1. && counter < NBQ) {
                outputs3.push_back(tree.closest_point_and_primitive(queries[counter]).second);
                ++counter;
        }
        timer.stop();
        speed = static_cast<double>(counter)/(counter == NBQ? timer.time(): 1.);
        std::cout << "with KD-tree:      " << speed << " queries/s" << std::endl << std::endl;
        std::cout << "Consistency:" << std::endl;
        if((counter = check_outputs(outputs1, outputs2, Id())) == 0)
                std::cout << "         without hint and spatial sort are consistent" << std::endl;
        else
                std::cout << "WARNING, without hint and spatial sort have " << counter << " inconsistencies (closest point on vertex/edge?)" << std::endl;

        if((counter = check_outputs(outputs1, outputs3, Id())) == 0)
                std::cout << "         without hint and with KD-tree are consistent (modulo hint case)" << std::endl;
        else
                std::cout << "WARNING, without hint and with KD-tree have " << counter << " inconsistencies (closest point on vertex/edge? the hint case has been excluded)" << std::endl;

        std::cout << std::endl;
}

template<class K, class Tree, class Polyhedron>
void test_impl(Tree& tree, Polyhedron& p)
{
  test_hint_strategies<Tree,K>(tree, p);
}

int main(void)
{
        std::cout << "AABB hint strategies tests" << std::endl;
        test_kernels<TRIANGLE>("./data/cube.off");
        test_kernels<TRIANGLE>("./data/coverrear.off");
        test_kernels<TRIANGLE>("./data/nested_spheres.off");
        test_kernels<TRIANGLE>("./data/finger.off");
        return 0;
}
