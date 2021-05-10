/*
 * Copyright (C) 2020 Paul Davis <paul@linuxaudiosystems.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include <iostream>

#include "canvas/item.h"
#include "canvas/types.h"
#include "canvas/constrained_item.h"

using namespace ArdourCanvas;

using std::cerr;
using std::endl;
using kiwi::Constraint;
using kiwi::Variable;

ConstrainedItem::ConstrainedItem (Item& i)
	: _item (i)
	, _left (_item.name + " left")
	, _right (_item.name + " right")
	, _top (_item.name + " top")
	, _bottom (_item.name + " bottom")
	, _width (_item.name + " width")
	, _height (_item.name + " height")
	, _left_padding (_item.name + " left_padding")
	, _right_padding (_item.name + " right_padding")
	, _top_padding (_item.name + " top_padding")
	, _bottom_padding (_item.name + " bottom_padding")
	, _center_x (_item.name + " center_x")
	, _center_y (_item.name + " center_y")
{
	/* setup center_{x,y} variables in case calling/using
	 * code wants to use them for additional constraints
	 */

	_constraints.push_back (center_x() == left() + (width() / 2.));
	_constraints.push_back (center_y() == top() + (height() / 2.));

	_constraints.push_back (_right == _left + _width);
	_constraints.push_back (_bottom == _top + _height);
}

ConstrainedItem::~ConstrainedItem ()
{
}

void
ConstrainedItem::constrained (ConstraintPacker const & parent)
{
	/* our variables should be set. Deliver computed size to item */

	Rect r (_left.value(), _top.value(), _right.value(), _bottom.value());
	//dump (cerr);
	// cerr << _item.whoami() << " constrained-alloc " << r << endl;

	_item.size_allocate (r);
}

void
ConstrainedItem::dump (std::ostream& out)
{
	out << _item.name << " value dump:\n"
	    << '\t' << "left: " << _left.value() << '\n'
	    << '\t' << "right: " << _right.value() << '\n'
	    << '\t' << "top: " << _top.value() << '\n'
	    << '\t' << "bottom: " << _bottom.value() << '\n'
	    << '\t' << "width: " << _width.value() << '\n'
	    << '\t' << "height: " << _height.value() << '\n'
	    << '\t' << "right_padding: " << _right_padding.value() << '\n'
	    << '\t' << "left_padding: " << _left_padding.value() << '\n'
	    << '\t' << "top_padding: " << _top_padding.value() << '\n'
	    << '\t' << "bottom_padding: " << _bottom_padding.value() << '\n'
	    << '\t' << "center_x: " << _center_x.value() << '\n'
	    << '\t' << "center_y: " << _center_y.value() << '\n';
}

bool
ConstrainedItem::involved (Constraint const & c) const
{
	if (c.involves (_left) ||
	    c.involves (_right) ||
	    c.involves (_top) ||
	    c.involves (_bottom) ||
	    c.involves (_width) ||
	    c.involves (_height) ||
	    c.involves (_center_x) ||
	    c.involves (_center_y)) {
		return true;
	}

	return false;
}

/*** BoxConstrainedItem */

BoxConstrainedItem::BoxConstrainedItem (Item& parent, PackOptions primary_axis_opts, PackOptions secondary_axis_opts)
	: ConstrainedItem (parent)
	, _left_margin (_item.name + " left_margin")
	, _right_margin (_item.name + " right_margin")
	, _top_margin (_item.name + " top_margin")
	, _bottom_margin (_item.name + " bottom_margin")
	, _primary_axis_pack_options (primary_axis_opts)
	, _secondary_axis_pack_options (secondary_axis_opts)
{
}

BoxConstrainedItem::~BoxConstrainedItem ()
{
}

bool
BoxConstrainedItem::involved (Constraint const & c) const
{
	if (ConstrainedItem::involved (c)) {
		return true;
	}

	if (c.involves (_left_margin) ||
	    c.involves (_right_margin) ||
	    c.involves (_top_margin) ||
	    c.involves (_bottom_margin)) {
		return true;
	}

	return false;
}

void
BoxConstrainedItem::dump (std::ostream& out)
{
	ConstrainedItem::dump (out);

	out << '\t' << "left_margin: " << _left_margin.value() << '\n'
	    << '\t' << "right_margin: " << _right_margin.value() << '\n'
	    << '\t' << "top_margin: " << _top_margin.value() << '\n'
	    << '\t' << "bottom_margin: " << _bottom_margin.value() << '\n';
}

ConstrainedItem&
ConstrainedItem::at (Duple const & d)
{
	_constraints.push_back (_left == d.x);
	_constraints.push_back (_top == d.y);

	return *this;
}

ConstrainedItem&
ConstrainedItem::size (Duple const & d)
{
	_constraints.push_back (_width == d.x);
	_constraints.push_back (_height == d.y);

	return *this;
}

ConstrainedItem&
ConstrainedItem::box (Rect const & r)
{
	_constraints.push_back (_left == r.x0);
	_constraints.push_back (_top == r.y0);
	_constraints.push_back (_width == r.width());
	_constraints.push_back (_height == r.height());

	return *this;
}


ConstrainedItem&
ConstrainedItem::left_of (ConstrainedItem const & other, Distance by)
{
	_constraints.push_back (_right_padding == by);
	_constraints.push_back (_right == other.left() + _right_padding);
	return *this;
}

ConstrainedItem&
ConstrainedItem::right_of (ConstrainedItem const & other, Distance by)
{
	_constraints.push_back (_left_padding == by);
	_constraints.push_back (_left == other.right() + _left_padding);
	return *this;
}

ConstrainedItem&
ConstrainedItem::above (ConstrainedItem const & other, Distance by)
{
	_constraints.push_back (_bottom_padding == by);
	_constraints.push_back (_bottom == other.top() + _bottom_padding);
	return *this;
}

ConstrainedItem&
ConstrainedItem::below (ConstrainedItem const & other, Distance by)
{
	_constraints.push_back (_top_padding == by);
	_constraints.push_back (_top == other.bottom() + _top_padding);
	return *this;
}

ConstrainedItem&
ConstrainedItem::centered_on (ConstrainedItem const & other, Distance xoffset, Distance yoffset)
{
	_constraints.push_back (_center_x == other.center_x() + xoffset);
	_constraints.push_back (_center_y == other.center_y() + yoffset);
	return *this;
}

ConstrainedItem&
ConstrainedItem::top_aligned_with (ConstrainedItem const & other, Distance offset)
{
	_constraints.push_back (_top == other.top() + offset);
	return *this;
}

ConstrainedItem&
ConstrainedItem::bottom_aligned_with (ConstrainedItem const & other, Distance offset)
{
	_constraints.push_back (_bottom == other.bottom() + offset);
	return *this;
}

ConstrainedItem&
ConstrainedItem::left_aligned_with (ConstrainedItem const & other, Distance offset)
{
	_constraints.push_back (_left == other.left() + offset);
	return *this;
}

ConstrainedItem&
ConstrainedItem::right_aligned_with (ConstrainedItem const & other, Distance offset)
{
	_constraints.push_back (_right == other.right() + offset);
	return *this;
}

ConstrainedItem&
ConstrainedItem::same_size_as (ConstrainedItem const & other, Distance wdelta, Distance hdelta)
{
	_constraints.push_back (_width == other.width() + wdelta);
	_constraints.push_back (_height == other.height() + hdelta);
	return *this;
}

ConstrainedItem&
ConstrainedItem::same_width_as (ConstrainedItem const & other, Distance delta)
{
	_constraints.push_back (_width == other.width() + delta);
	return *this;
}

ConstrainedItem&
ConstrainedItem::same_height_as (ConstrainedItem const & other, Distance delta)
{
	_constraints.push_back (_height == other.height() + delta);
	return *this;
}

