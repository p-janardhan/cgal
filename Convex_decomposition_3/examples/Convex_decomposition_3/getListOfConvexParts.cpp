// Copyright (c) 2005-2008 Max-Planck-Institute Saarbruecken (Germany).
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
// $URL: 
// $Id: 
// 
//
// Author(s)     :  Peter Hachenberger <hachenberger@mpi-sb.mpg.de>

#include <CGAL/basic.h>
#include <CGAL/Exact_predicates_exact_constructions_kernel.h>
#include <CGAL/Polyhedron_3.h>
#include <CGAL/Nef_polyhedron_3.h>
#include <CGAL/IO/Polyhedron_iostream.h>
#include<CGAL/Nef_3/SNC_indexed_items.h>
#include <CGAL/convex_decomposition_3.h> 
#include <list>

typedef CGAL::Exact_predicates_exact_constructions_kernel Kernel;
typedef CGAL::Polyhedron_3<Kernel> Polyhedron_3;
typedef CGAL::Nef_polyhedron_3<Kernel, CGAL::SNC_indexed_items> Nef_polyhedron_3;
typedef Nef_polyhedron_3::Volume_const_iterator Volume_const_iterator;

int main(int argc, char* argv[]) {
  
  if(argc!=2) {
    std::cerr << "syntax: ./getListOfConvexParts <off-file>" << std::endl;
    return 1;
  }

  std::ifstream in(argv[1]);
  Polyhedron_3 Pin;
  in >> Pin;
  Nef_polyhedron_3 N(Pin);

  CGAL::convex_decomposition_3<Nef_polyhedron_3>(N);
  std::list<Polyhedron_3> convex_parts;
  
  Volume_const_iterator ci;
  for(ci = ++N.volumes_begin(); ci != N.volumes_end(); ++ci) {
    if(ci->mark()) {
      Polyhedron_3 P;
      N.convert_inner_shell_to_polyhedron(ci->shells_begin(), P);
      convex_parts.push_back(P);
    }
  }
  std::cout << "decomposition into " << convex_parts.size() << " convex parts " << std::endl;

}
