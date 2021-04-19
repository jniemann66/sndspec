/*
* Copyright (C) 2019 - 2021 Judd Niemann - All Rights Reserved.
* You may use, distribute and modify this code under the
* terms of the GNU Lesser General Public License, version 2.1
*
* You should have received a copy of GNU Lesser General Public License v2.1
* with this file. If not, please refer to: https://github.com/jniemann66/ReSampler
*/

#include "tests.h"
#include "window.h"

bool tests::testWindow()
{
	Sndspec::Window<double> w;
	w.generate("triangular", 9, 20);
	for(double v : w.getData())
	{
		std::cout << v << std::endl;
	}
}
