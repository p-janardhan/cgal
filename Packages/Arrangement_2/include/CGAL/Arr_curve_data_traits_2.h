// Copyright (c) 2005  Tel-Aviv University (Israel).
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
// $Source$
// $Revision$ $Date$
// $Name$
//
// Author(s)     : Ron Wein          <wein@post.tau.ac.il>
//                 Efi Fogel         <efif@post.tau.ac.il>
#ifndef CGAL_ARR_CURVE_DATA_TRAITS_2_H
#define CGAL_ARR_CURVE_DATA_TRAITS_2_H

/*! \file
 * Definition of the Arr_curve_data_traits_2 class.
 */

#include<list>

CGAL_BEGIN_NAMESPACE

/*!
 * \struct A simple convertor from one type to another.
 */
template <class TYPE_FROM, class TYPE_TO>
struct _Simple_convert_func
{
  TYPE_TO operator() (const TYPE_FROM& obj)
  {
    return (obj);
  }
};

/*! \class
 * A generic traits class for maintaining an arrangement of curves that have
 * an extra data field. This traits class is templated with an ordinary traits
 * class, which is also used as a based traits class to inherit from.
 * It can attach data objects to Curve_2 and to X_monotone_curve_2 objects
 * (possibly of two different types).
 * The data field is updated when the curves are converted from Curve_2 to
 * X_monotone_curve_2, and when the X_monotone_curve_2 curves are split.
 * When two x-monotone curves overlap, the data field to be associated with
 * the overlapping subcurve is obtained from the merge functor.
 * All other functors are inherited from the base ordinary traits class.
 */
template <class Traits_, class XMonotoneCurveData_, class MergeFunc_,
          class CurveData_ = XMonotoneCurveData_,
          class ConvertFunc_ = _Simple_convert_func<CurveData_,
                                                    XMonotoneCurveData_> >
class Arr_curve_data_traits_2 : public Traits_ 
{
public:

  typedef Traits_                                   Base_traits;
  typedef XMonotoneCurveData_                       X_monotone_curve_data;
  typedef MergeFunc_                                Merge_functor;
  typedef CurveData_                                Curve_data;
  typedef ConvertFunc_                              Convert_functor;
  
  typedef typename Base_traits::Curve_2             Base_curve_2;
  typedef typename Base_traits::X_monotone_curve_2  Base_x_monotone_curve_2;
  typedef typename Base_traits::Point_2             Point_2;

  typedef typename Base_traits::Has_left_category   Has_left_category;
  typedef typename Base_traits::Has_merge_category  Base_has_merge_category;
  typedef Tag_true                                  Has_merge_category;

  /*!
   * Representation of an input curve with an addtional data field.
   */
  class Curve_2 : public Base_curve_2 
  {
  private:
    Curve_data   m_data;

  public:

    /*!
     * Default constructor.
     */
    Curve_2 ()
    {}
    
    /*!
     * Construct a curve from an original curve and a data object.
     * \param cv The original curve.
     * \param data The data object.
     */ 
    Curve_2 (const Base_curve_2& cv, const Curve_data& data) :
      Base_curve_2 (cv),
      m_data (data)
    {}

    /*!
     * Get the data (const version).
     * \return The data object associated with the curve.
     */
    const Curve_data& data () const
    {
      return (m_data);
    }

    /*!
     * Get the data (non-const version).
     * \return The data object associated with the curve.
     */
    Curve_data& data ()
    {
      return (m_data);
    }

    /*!
     * Set the curve data.
     * \param data The data object to be associated with the curve.
     */
    void set_data (const Curve_data& data)
    {
      m_data = data;
      return;
    }
  };
  
  /*!
   * Representation of an input curve with an addtional data field.
   */
  class X_monotone_curve_2 : public Base_x_monotone_curve_2 
  {
  private:
    X_monotone_curve_data  m_data;

  public:

    /*!
     * Default constructor.
     */
    X_monotone_curve_2()
    {}
    
    /*!
     * Construct a curve from an original x-monotone curve and a data object.
     * \param cv The original x-monotone curve.
     * \param data The data object.
     */ 
    X_monotone_curve_2 (const Base_x_monotone_curve_2& cv,
                        const X_monotone_curve_data& data) :
      Base_x_monotone_curve_2 (cv),
      m_data (data)
    {}

    /*!
     * Construct a curve from an original x-monotone curve.
     * \param cv The original x-monotone curve.
     */ 
    X_monotone_curve_2 (const Base_x_monotone_curve_2& cv) :
      Base_x_monotone_curve_2 (cv),
      m_data ()
    {}

    /*!
     * Get the data object associated with the curve (const version).
     */
    const X_monotone_curve_data& data () const
    {
      return (m_data);
    }

    /*!
     * Get the data object associated with the curve (non-const version).
     */
    X_monotone_curve_data& data ()
    {
      return (m_data);
    }

    /*!
     * Check whether the given curve has the same data.
     */
    bool has_same_data (const X_monotone_curve_2& cv) const
    {
      return (m_data == cv.m_data);
    }

    /*!
     * Set a data object to the curve.
     * \param data The data object to set.
     */
    void set_data (const X_monotone_curve_data& data)
    {
      m_data = data;
      return;
    }

  };

public:
  
  /// \name Construction.
  //@{

  /*! Default constructor. */
  Arr_curve_data_traits_2 ()
  {}
  
  /*! Constructor from a base-traits class. */
  Arr_curve_data_traits_2 (const Base_traits& traits) :
    Base_traits (traits)
  {}
  //@}

  /// \name Overriden functors.
  //@{

  class Make_x_monotone_2
  {
  private:
    Base_traits    *base;

  public:

    /*! Constructor. */
    Make_x_monotone_2 (Base_traits *_base) :
      base (_base)
    {}
    
    /*!
     * Cut the given curve into x-monotone subcurves and insert them to the
     * given output iterator. As segments are always x_monotone, only one
     * x-monotone curve will be contained in the iterator.
     * \param cv The curve.
     * \param oi The output iterator, whose value-type is X_monotone_curve_2.
     * \return The past-the-end iterator.
     */
    template<class OutputIterator>
    OutputIterator operator() (const Curve_2& cv, OutputIterator oi)
    {
      // Make the original curve x-monotone.
      std::list<CGAL::Object>       base_objects;
    
      base->make_x_monotone_2_object() (cv,
                                        std::back_inserter (base_objects));

      // Attach the data to each of the resulting x-monotone curves.
      typename std::list<CGAL::Object>::const_iterator  it;
      const Base_x_monotone_curve_2  *base_x_curve;
      X_monotone_curve_data           xdata = Convert_functor()(cv.data());

      for (it = base_objects.begin(); it != base_objects.end(); ++it)
      {
        base_x_curve = object_cast<Base_x_monotone_curve_2> (&(*it));
        if (base_x_curve != NULL)
        {
          // Current object is an x-monotone curve: Attach data to it.
          *oi = make_object (X_monotone_curve_2 (*base_x_curve,
                                                 xdata));
        }
        else
        {
          // Current object is an isolated point: Leave it as is.
          CGAL_assertion (object_cast<Point_2> (&(*it)) != NULL);
          *oi = *it;
        }
        ++oi;
      }

      return (oi);
    }
  };

  /*! Get a Make_x_monotone_2 functor object. */
  Make_x_monotone_2 make_x_monotone_2_object ()
  {
    return Make_x_monotone_2 (this);
  }

  class Split_2
  {
  private:
    Base_traits    *base;

  public:

    /*! Constructor. */
    Split_2 (Base_traits *_base) :
      base (_base)
    {}

    /*!
     * Split a given x-monotone curve at a given point into two sub-curves.
     * \param cv The curve to split
     * \param p The split point.
     * \param c1 Output: The left resulting subcurve (p is its right endpoint).
     * \param c2 Output: The right resulting subcurve (p is its left endpoint).
     * \pre p lies on cv but is not one of its end-points.
     */
    void operator() (const X_monotone_curve_2& cv, const Point_2 & p,
                     X_monotone_curve_2& c1, X_monotone_curve_2& c2)
    {
      // Split the original curve.
      base->split_2_object() (cv, p,
                              c1, c2);

      // Attach data to the split curves.
      c1.set_data (cv.data());
      c2.set_data (cv.data());

      return;
    }
  };

  /*! Get a Split_2 functor object. */
  Split_2 split_2_object ()
  {
    return Split_2 (this);
  }

  class Intersect_2
  {
  private:
    Base_traits    *base;

  public:

    /*! Constructor. */
    Intersect_2 (Base_traits *_base) :
      base (_base)
    {}

    /*!
     * Find the intersections of the two given curves and insert them to the
     * given output iterator. As two segments may itersect only once, only a
     * single will be contained in the iterator.
     * \param cv1 The first curve.
     * \param cv2 The second curve.
     * \param oi The output iterator.
     * \return The past-the-end iterator.
     */
    template<class OutputIterator>
    OutputIterator operator() (const X_monotone_curve_2& cv1,
                               const X_monotone_curve_2& cv2,
                               OutputIterator oi)
    {
      // Use the base functor to obtain all intersection objects.
      std::list<CGAL::Object>                   base_objects;

      base->intersect_2_object() (cv1, cv2,
                                  std::back_inserter (base_objects));

      // Stop if the list is empty:
      if (base_objects.empty())
        return (oi);

      // Go over all intersection objects and prepare the output.
      typename std::list<CGAL::Object>::const_iterator  it;
      const Base_x_monotone_curve_2                    *base_cv;

      for (it = base_objects.begin(); it != base_objects.end(); ++it)
      {
        if ((base_cv = object_cast<Base_x_monotone_curve_2> (&(*it))) != NULL)
        {
          // The current intersection object is an overlapping x-monotone
          // curve: Merge the data fields of both intersecting curves and
          // associate the result with the overlapping curve.
          X_monotone_curve_2  cv (*base_cv,
                                  Merge_functor() (cv1.data(),
                                                   cv2.data()));

          *oi = make_object (cv);
        }
        else
        {
          // The current intersection object is an intersection point:
          // Copy it as is.
          *oi = *it;
        }
        ++oi;
      }

      return (oi);
    }
  };

  /*! Get an Intersect_2 functor object. */
  Intersect_2 intersect_2_object ()
  {
    return Intersect_2 (this);
  }

  class Are_mergeable_2
  {
  private:
    const Base_traits    *base;

  public:

    /*! Constructor. */
    Are_mergeable_2 (const Base_traits *_base) :
      base (_base)
    {}

    /*!
     * Check whether it is possible to merge two given x-monotone curves.
     * \param cv1 The first curve.
     * \param cv2 The second curve.
     * \return (true) if the two curves are mergeable; (false) otherwise.
     */
    bool operator() (const X_monotone_curve_2& cv1,
                     const X_monotone_curve_2& cv2) const
    {
      // In case the two base curves are not mergeable, the extended curves
      // are not mergeable as well.
      if (!_are_mergeable_base_imp (cv1, cv2,
                                    Base_has_merge_category()))
      {
        return (false);
      }

      // Make sure that the data attached to both curves is the same.
      return (cv1.has_same_data (cv2));
    }

  private:

    /*!
     * Implementation of the base predicate in case the HasMerge tag is true.
     */
    bool _are_mergeable_base_imp (const X_monotone_curve_2& cv1,
                                  const X_monotone_curve_2& cv2,
                                  Tag_true) const
    {
      return (base->are_mergeable_2_object() (cv1, cv2));
    }

    /*!
     * Implementation of the base predicate in case the HasMerge tag is false.
     */
    bool _are_mergeable_base_imp (const X_monotone_curve_2& ,
                                  const X_monotone_curve_2& ,
                                  Tag_false) const
    {
      // Curve merging is not supported:
      return (false);
    }
  };
  
  /*! Get an Are_mergeable_2 functor object. */
  Are_mergeable_2 are_mergeable_2_object () const
  {
    return Are_mergeable_2 (this);
  }

  class Merge_2
  {
  private:
    Base_traits    *base;

  public:

    /*! Constructor. */
    Merge_2 (Base_traits *_base) :
      base (_base)
    {}

    /*!
     * Merge two given x-monotone curves into a single curve (segment).
     * \param cv1 The first curve.
     * \param cv2 The second curve.
     * \param c Output: The merged curve.
     * \pre The two curves are mergeable.
     */
    void operator() (const X_monotone_curve_2& cv1,
                     const X_monotone_curve_2& cv2,
                     X_monotone_curve_2& c)
    {
      // The function is implemented based on the base Has_merge category.
      _merge_imp (cv1, cv2, c, Base_has_merge_category());
    }

  private:

    /*!
     * Implementation of the operator() in case the HasMerge tag is true.
     */
    void _merge_imp (const X_monotone_curve_2& cv1,
                     const X_monotone_curve_2& cv2,
                     X_monotone_curve_2& c,
                     Tag_true)
    {      
      // Merge the two base curve.
      Base_x_monotone_curve_2  base_cv;

      base->merge_2_object() (cv1, cv2,
                              base_cv);

      // Attach data from one of the curves.
      CGAL_precondition (cv1.has_same_data (cv2));

      c = X_monotone_curve_2 (base_cv, cv1.data());
      return;
    }

    /*!
     * Implementation of the operator() in case the HasMerge tag is false.
     */
    void _merge_imp (const X_monotone_curve_2& ,
                     const X_monotone_curve_2& ,
                     X_monotone_curve_2& ,
                     Tag_false) const
    {
      // This function should never be called!
      CGAL_assertion_msg (false,
                          "Merging curves is not supported.");
    }
  };

  /*! Get a Merge_2 functor object. */
  Merge_2 merge_2_object ()
  {
    return Merge_2 (this);
  }

  class Construct_x_monotone_curve_2
  {
  private:
    const Base_traits    *base;

  public:

    /*! Constructor. */
    Construct_x_monotone_curve_2 (const Base_traits *_base) :
      base (_base)
    {}

    /*!
     * Return an x-monotone curve connecting the two given endpoints.
     * \param p The first point.
     * \param q The second point.
     * \pre p and q must not be the same.
     * \return An x-monotone curve connecting p and q.
     */
    X_monotone_curve_2 operator() (const Point_2& p,
                                   const Point_2& q) const
    {
      Base_x_monotone_curve_2  base_cv =
        base->construct_x_monotone_curve_2_object() (p, q);

      return (X_monotone_curve_2 (base_cv, X_monotone_curve_data()));
    }
  };

  /*! Get a Construct_x_monotone_curve_2 functor object. */
  Construct_x_monotone_curve_2 construct_x_monotone_curve_2_object () const
  {
    return Construct_x_monotone_curve_2 (this);
  }
  //@}

};

CGAL_END_NAMESPACE

#endif

