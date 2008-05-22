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
#include <CGAL/Nef_polyhedron_3.h>
#include <CGAL/IO/Nef_polyhedron_iostream_3.h>
#include <CGAL/IO/Qt_widget_Nef_3.h>
#include <qapplication.h>
#include <CGAL/convex_decomposition_3.h> 

#include <algorithm>
#include <map>
#include <iostream>
#include <fstream>

typedef CGAL::Exact_predicates_exact_constructions_kernel Kernel;

#include<CGAL/Nef_3/SNC_indexed_items.h>
typedef CGAL::Nef_polyhedron_3<Kernel, CGAL::SNC_indexed_items>     Nef_polyhedron_3;

int main(int argc, char* argv[]) {
  
  if(argc!=2) {
    std::cerr << "syntax: ./decomposition <nef3-file>" << std::endl;
    return 1;
  }

  std::ifstream in(argv[1]);
  Nef_polyhedron_3 N;
  in >> N;

  CGAL::convex_decomposition_3<Nef_polyhedron_3>(N);

  QApplication a(argc, argv);
  CGAL::Qt_widget_Nef_3<Nef_polyhedron_3>* w = 
    new CGAL::Qt_widget_Nef_3<Nef_polyhedron_3>(N);
  a.setMainWidget(w);
  w->show();
  a.exec();
}
