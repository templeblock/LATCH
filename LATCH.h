/*******************************************************************
*   LATCH.h
*   LATCH
*
*	Author: Kareem Omar
*	kareem.omar@uah.edu
*	https://github.com/komrad36
*
*	Last updated Sep 12, 2016
*******************************************************************/
//
// Fastest implementation of the fully scale-
// and rotation-invariant LATCH 512-bit binary
// feature descriptor as described in the 2015
// paper by Levi and Hassner:
//
// "LATCH: Learned Arrangements of Three Patch Codes"
// http://arxiv.org/abs/1501.03719
//
// See also the ECCV 2016 Descriptor Workshop paper, of which I am a coauthor:
//
// "The CUDA LATCH Binary Descriptor"
// http://arxiv.org/abs/1609.03986
//
// And the original LATCH project's website:
// http://www.openu.ac.il/home/hassner/projects/LATCH/
//
// See my GitHub for the CUDA version, which is extremely fast.
//
// My implementation uses multithreading, SSE2/3, and 
// many many careful optimizations to implement the
// algorithm as described in the paper, but at great speed.
// This implementation outperforms the reference implementation by 600%
// single-threaded or 2200% multi-threaded (!) while exactly matching
// the reference implementation's output and capabilities.
//
// All functionality is contained in the file LATCH.h.
// 'main.cpp' is simply a sample test harness with example usage and
// performance testing.
//

#pragma once

#include <future>
#include <immintrin.h>
#include <thread>
#include <vector>

// angle in RADIANS
struct KeyPoint {
	float x, y, scale;
	
	// RADIANS
	float angle;

	// angle in RADIANS
	KeyPoint(const float _x, const float _y, const float _scale, const float _angle) : x(_x), y(_y), scale(_scale), angle(_angle) {}
};

constexpr float triplets[3073] = {
	-5,-16,-9,1,16,-21,
	-7,-10,-3,16,-14,9,
	11,7,3,6,15,-11,
	-22,-12,-22,2,12,21,
	17,2,14,-10,10,-18,
	22,-2,23,-14,-20,5,
	4,24,4,16,-11,-21,
	13,19,23,-6,19,-4,
	22,1,-10,7,-8,6,
	-5,19,-6,18,11,-16,
	12,24,20,-9,20,-20,
	-3,5,10,-14,-21,19,
	-3,-17,-5,9,-1,-19,
	7,22,13,-2,-23,20,
	19,-1,15,12,-19,-9,
	-19,-2,-22,2,-9,24,
	-2,-11,-3,-21,-18,7,
	23,4,17,8,7,-19,
	16,10,20,18,-23,4,
	-6,-1,6,22,-11,-14,
	-24,15,-23,-15,-14,-11,
	24,18,14,-13,-14,-19,
	-17,-11,-21,10,-12,-17,
	19,3,-2,-4,8,19,
	-18,-8,-18,-3,-15,5,
	-6,1,-6,-13,16,23,
	-22,-4,-15,-8,-5,20,
	9,-6,10,13,-23,15,
	20,11,9,12,-24,22,
	-19,-1,-20,-4,5,2,
	-24,15,-19,4,-16,-5,
	-23,-11,-16,10,5,19,
	-3,-22,-7,5,23,-22,
	-15,-9,-21,2,21,20,
	-18,16,-12,11,-10,4,
	2,14,6,-7,-13,20,
	3,23,5,15,-23,-9,
	1,-1,-3,8,-4,7,
	-22,-1,16,16,19,-12,
	14,9,19,-12,13,12,
	21,-22,15,2,-17,3,
	0,11,8,5,-22,18,
	-15,13,-18,9,3,-17,
	-12,8,-21,10,5,-18,
	19,0,-12,17,18,-7,
	-14,1,-12,-20,-7,-2,
	24,3,-16,15,-22,7,
	-17,24,-24,13,-14,-24,
	-7,-4,-1,-23,10,20,
	-8,-1,-5,-13,20,-22,
	13,16,0,-22,16,-14,
	-10,0,-9,12,-9,-16,
	20,-2,19,15,-19,-15,
	-19,22,-14,19,23,-2,
	-24,10,-8,16,-3,-1,
	-22,18,-23,-15,23,21,
	9,9,21,10,-23,-24,
	-17,12,-12,-19,-23,-8,
	19,0,-18,17,-16,-19,
	-19,-7,-14,21,20,3,
	20,8,17,-2,0,-10,
	-15,6,-21,24,2,-22,
	-18,8,-24,12,19,-14,
	16,-13,24,-1,-16,-18,
	23,4,20,-1,-10,22,
	22,2,16,-14,-22,11,
	-14,24,-9,-19,-16,-6,
	11,0,-20,-5,24,-12,
	17,-9,18,16,1,-15,
	15,-24,24,-10,16,11,
	5,-3,4,3,-16,-2,
	9,-15,7,19,-18,-19,
	11,-5,14,6,22,-10,
	-8,17,-19,-3,24,17,
	13,24,9,16,-20,-4,
	7,16,8,-6,11,1,
	3,18,20,23,15,-24,
	22,2,-12,15,18,-10,
	-15,11,-13,13,9,9,
	-10,17,-13,-9,13,-13,
	22,-11,20,15,7,-9,
	18,-11,17,0,16,11,
	22,-19,24,-18,-3,-15,
	22,-5,19,20,16,-23,
	21,0,-17,14,-8,2,
	9,2,7,0,3,12,
	7,-12,13,18,8,-24,
	-5,-15,-22,-11,-10,9,
	19,20,24,24,6,-11,
	-10,2,-17,6,-2,-21,
	19,21,21,-17,21,-13,
	-11,-20,-9,3,-5,-8,
	-3,-7,-1,-20,23,-9,
	-3,22,-1,21,-12,3,
	-19,2,-24,-4,-3,17,
	-16,2,-19,10,1,20,
	-9,-20,-17,21,-17,22,
	-18,-9,-12,7,19,-5,
	22,8,11,6,-7,22,
	-24,5,-24,10,-15,-20,
	10,-22,9,20,-4,-20,
	-24,0,-23,6,-13,-15,
	22,-3,19,-20,24,5,
	10,-1,8,-23,-9,-5,
	-9,23,-22,-6,0,13,
	-9,23,-17,-7,-13,-20,
	20,-21,22,7,15,-5,
	24,-24,12,24,22,-12,
	16,-18,9,-13,11,-6,
	-10,2,18,-9,15,18,
	-18,1,-24,3,24,20,
	-23,-22,-17,6,-12,7,
	-19,-18,-2,-18,22,13,
	-22,4,-22,24,8,2,
	7,10,20,-12,19,21,
	-18,3,-16,-7,8,5,
	-20,4,-17,-14,-9,-8,
	-7,-20,-20,-6,18,-3,
	0,4,18,4,-8,-8,
	-15,1,-17,-18,-22,10,
	8,21,24,13,17,-3,
	14,6,17,10,-24,-10,
	-22,12,-18,8,-8,23,
	-13,21,-15,-8,18,11,
	20,7,5,17,7,-24,
	-7,1,9,-12,14,17,
	16,-24,24,11,-2,21,
	10,-15,21,9,15,16,
	-10,14,-22,3,-11,8,
	22,-22,14,-13,4,3,
	-1,23,-2,21,-14,-17,
	16,-7,19,-7,-23,6,
	21,-15,6,-22,6,-4,
	11,-1,-10,7,-21,-4,
	-10,-2,-13,-23,23,5,
	-21,18,-8,-24,11,5,
	-18,23,-21,-24,-16,16,
	-8,19,-23,6,-13,23,
	-24,-16,-1,-23,13,-11,
	-23,12,-24,-10,1,2,
	-14,2,-20,-20,-11,24,
	-1,20,-6,-4,-22,-19,
	12,13,15,2,-16,1,
	11,-24,24,13,10,21,
	-10,8,-13,-12,10,24,
	23,-13,16,10,-14,10,
	0,-21,0,22,20,18,
	24,-13,18,7,7,19,
	-10,-24,-13,-24,-20,-5,
	19,0,21,-4,21,16,
	0,-13,-2,-15,24,9,
	-12,-10,-6,19,6,-16,
	-10,-4,-14,4,-22,-8,
	24,-18,20,-7,24,-1,
	18,-8,23,7,3,-11,
	-20,-11,-5,-6,17,-19,
	12,1,12,0,-11,-16,
	22,-2,-24,-1,-3,22,
	17,-7,17,17,17,-12,
	-22,-17,-6,-6,14,11,
	-11,-4,-2,-13,-9,-18,
	-24,12,-16,16,-5,13,
	-20,15,-14,23,-10,-16,
	21,-18,12,13,0,-12,
	-18,-20,-18,14,-22,12,
	-16,3,-17,21,-3,-15,
	12,15,5,15,-19,4,
	15,-16,17,-8,20,23,
	-21,7,-7,19,-2,1,
	-22,21,-21,13,19,-10,
	9,21,6,0,-23,-24,
	-24,4,-24,13,16,-13,
	-8,15,-11,-18,23,23,
	18,2,15,10,23,6,
	-14,22,-21,-14,9,17,
	3,-24,-2,-12,0,-3,
	18,-9,19,18,7,17,
	20,-20,11,-11,0,4,
	5,11,18,-7,-22,-5,
	-15,24,-17,-23,22,18,
	-23,-15,-7,-5,-14,11,
	-3,3,-4,-7,17,-13,
	19,-2,17,-19,24,1,
	2,8,11,-21,20,-12,
	22,-9,8,11,-14,-4,
	-19,20,-22,2,-14,23,
	9,-24,9,-17,-22,17,
	6,-1,-4,-21,0,18,
	8,-9,12,-13,-23,13,
	7,-13,16,-2,5,-5,
	-19,9,-10,-23,23,9,
	1,24,4,-17,-24,3,
	10,-2,12,-5,-1,15,
	22,6,-6,11,20,-9,
	24,8,-7,-22,23,-10,
	3,5,2,8,-6,14,
	13,-10,7,-12,11,11,
	16,-7,21,-13,-7,-12,
	9,14,5,-8,18,-9,
	12,-2,8,-22,13,21,
	3,-3,-10,-1,-2,19,
	-13,7,-21,-8,-5,-17,
	16,-4,18,-14,-22,17,
	13,23,12,2,-2,5,
	-12,-4,-14,1,9,-2,
	-17,6,-12,22,-9,-13,
	-9,22,-24,2,-1,-10,
	-14,-23,-13,14,6,-3,
	-23,7,-23,-12,12,16,
	-6,4,3,20,22,-20,
	5,14,3,-15,12,-13,
	19,-11,24,-22,-12,-20,
	9,-12,16,17,21,24,
	-20,-10,-14,-9,16,-21,
	-7,-23,-4,-20,10,-8,
	18,-14,20,6,24,10,
	-2,4,10,-21,-16,-2,
	-18,-11,-15,-12,14,-21,
	14,-15,18,-7,-17,-2,
	12,-1,20,18,0,7,
	-13,-12,-17,14,23,18,
	21,-5,24,-7,-15,-6,
	1,16,-5,1,9,18,
	22,-4,22,3,18,8,
	-10,6,-16,9,4,-24,
	5,-4,-18,-18,-12,-24,
	-16,-6,-19,-19,18,23,
	19,-23,23,-4,-10,-13,
	13,-6,19,8,19,-6,
	8,-17,12,-17,0,22,
	8,7,22,22,-22,-22,
	22,-19,19,-23,-17,16,
	0,21,13,12,21,-22,
	-19,-15,-12,3,-16,1,
	-1,22,-1,-24,19,13,
	-9,14,-6,7,24,-1,
	-18,-11,-18,-3,-22,-5,
	-13,1,15,9,18,7,
	-5,10,24,12,-24,22,
	-7,0,-1,-15,3,24,
	21,-23,19,-16,-5,8,
	18,-12,14,18,-16,-5,
	-20,16,-20,19,-2,-19,
	-20,22,-23,-13,21,-3,
	19,-16,23,-19,24,7,
	-19,-1,18,5,20,15,
	13,22,24,-11,3,-14,
	-19,14,-23,-9,12,17,
	-6,14,-9,-1,-22,-9,
	19,-1,-21,-18,23,-10,
	22,17,14,1,-12,-23,
	7,0,-3,-8,8,21,
	24,-2,24,2,-6,-12,
	8,2,9,14,-3,6,
	-10,-3,-11,-24,18,-5,
	-12,-20,-7,12,-5,-6,
	-17,-2,-17,-21,-15,-15,
	5,21,9,-15,-23,-12,
	-10,10,-8,10,4,7,
	23,-3,9,4,11,4,
	-1,-9,-21,-21,22,-4,
	-12,19,-15,-3,-24,0,
	-24,-8,-19,9,17,-7,
	20,-16,21,-24,-11,19,
	11,-21,19,-14,23,-21,
	23,-9,20,7,17,-6,
	-18,-5,5,21,-17,11,
	-3,-15,-21,-16,-13,11,
	14,-2,-20,3,2,-17,
	-16,19,-12,-21,-3,15,
	21,19,5,20,-6,-1,
	21,9,1,12,23,-9,
	9,-6,22,7,6,0,
	4,14,23,-21,-4,18,
	-6,-12,-23,-24,-13,9,
	14,22,6,21,-19,-9,
	-2,-8,-5,22,-16,23,
	-3,23,-3,19,10,-18,
	-22,7,-19,21,12,-12,
	-9,23,-16,-6,-24,7,
	1,6,14,21,15,-3,
	4,-2,-11,18,10,20,
	3,23,23,-20,-13,23,
	11,21,20,-12,19,-8,
	14,14,13,-4,22,-14,
	14,23,10,-12,7,21,
	2,4,11,-20,-13,-11,
	14,8,16,-13,24,-14,
	-9,17,-13,-8,-3,21,
	-17,4,-17,16,0,24,
	-22,-15,-1,18,-24,8,
	-16,-7,-14,-10,22,5,
	1,11,-1,-24,-9,10,
	7,-12,14,9,-4,18,
	21,15,5,-3,-23,-5,
	16,8,18,24,-11,23,
	17,-1,18,-12,13,19,
	-18,10,-14,12,-14,24,
	-4,-5,-9,20,12,20,
	16,-5,15,-8,-20,-7,
	-24,5,-16,10,-11,-6,
	-21,12,-24,-15,14,-15,
	21,-1,19,6,-5,19,
	24,12,3,10,12,-15,
	-22,1,-17,-13,-4,4,
	19,-8,22,-12,14,-23,
	11,-9,16,-4,-10,2,
	-24,-8,5,21,-21,14,
	14,-21,22,-11,-8,-22,
	-23,-1,19,-5,-11,13,
	13,5,4,-12,18,-24,
	-13,2,-14,22,9,19,
	13,-15,9,22,19,-17,
	-3,4,-3,5,-2,-14,
	11,2,4,-21,-15,2,
	-17,-5,-16,-1,15,-20,
	15,0,7,-7,-20,0,
	-2,23,-1,12,-19,-10,
	-3,13,19,-18,15,-23,
	-21,5,-19,10,3,18,
	-17,9,-16,-12,19,24,
	-20,-1,-24,-24,-22,-9,
	12,-16,18,-5,-19,-6,
	-12,10,-8,-23,8,-11,
	17,-8,15,21,-13,11,
	11,24,11,14,-23,6,
	15,7,19,22,-23,14,
	-4,21,-2,18,7,5,
	23,24,10,10,-24,10,
	-14,-4,-21,24,3,2,
	2,24,6,-6,7,24,
	14,-16,18,-8,-15,11,
	-21,-8,-20,0,14,23,
	-23,11,-24,22,7,22,
	7,24,9,19,0,23,
	14,-6,20,-9,7,-23,
	20,9,13,1,9,21,
	-11,-1,9,-11,15,23,
	8,-9,23,2,10,21,
	-19,4,-19,-22,-2,20,
	14,20,19,-23,1,23,
	12,-1,-15,22,13,-18,
	24,-5,21,11,6,12,
	11,-2,12,5,15,7,
	-16,10,-15,-13,10,17,
	-18,-15,-19,-5,3,8,
	12,12,14,-9,20,-12,
	-13,-23,-22,-18,-8,13,
	15,-22,16,-18,18,14,
	7,23,20,-9,2,-16,
	-17,-7,-24,-23,9,23,
	9,-5,5,-22,-16,-11,
	-22,-11,-20,11,-2,14,
	11,8,16,-24,18,-14,
	17,1,-6,12,7,18,
	4,8,7,-19,4,18,
	-10,-1,-19,19,-23,-23,
	-1,23,1,5,13,-24,
	-16,14,-19,-19,21,-22,
	-9,-23,-9,5,16,3,
	12,21,13,11,15,23,
	-21,23,-21,-12,-24,-17,
	-14,-8,-5,-21,13,-1,
	17,24,10,19,-4,6,
	21,-24,13,-22,-2,-8,
	23,-16,21,-6,-5,-15,
	-12,14,-11,-8,-6,5,
	-5,9,16,9,-23,24,
	12,-24,22,8,19,2,
	-10,0,18,23,3,-22,
	-19,3,-12,-20,15,6,
	-23,19,-23,5,-3,23,
	24,8,4,10,24,-3,
	8,15,7,-23,-2,20,
	19,18,23,4,-24,6,
	-9,-23,-6,16,-2,2,
	10,7,12,-1,21,7,
	-6,2,20,16,5,6,
	11,3,-4,-14,9,-12,
	13,-17,11,-22,23,15,
	11,1,17,22,4,-3,
	18,-9,21,11,-14,-23,
	-9,14,-14,-3,14,4,
	12,-4,21,0,-24,16,
	-21,6,-16,-15,-23,-20,
	14,-22,0,-20,-13,1,
	7,-12,13,0,-15,-11,
	16,0,10,17,-1,21,
	14,2,-9,-18,24,-5,
	-12,23,-10,-6,8,9,
	16,0,-14,24,-10,-16,
	-21,-14,-23,12,-21,13,
	-21,21,-21,-24,2,-15,
	1,19,13,8,8,-23,
	-8,6,17,-19,-24,7,
	-24,18,-21,10,20,-24,
	3,11,-4,0,22,-12,
	8,5,18,-3,17,10,
	-21,18,-21,18,-22,-6,
	-9,2,-5,24,-17,4,
	2,6,5,5,-24,21,
	20,-24,5,-17,16,-3,
	-10,-2,5,5,-22,-9,
	-3,-13,-21,-12,4,23,
	6,8,3,24,13,-8,
	5,19,11,-6,10,22,
	-18,17,-18,-10,-18,10,
	20,21,21,17,-3,18,
	-23,15,-19,2,-21,-10,
	-16,22,-18,-18,2,22,
	2,0,5,19,-8,7,
	5,-21,4,4,-14,2,
	-16,1,18,19,15,4,
	-15,-2,-20,-6,-18,-8,
	-19,-7,0,-21,10,-23,
	-15,1,-16,22,7,0,
	20,12,22,-12,18,1,
	23,14,13,-2,23,-2,
	5,16,3,-1,-7,9,
	-11,11,-13,-11,-20,-23,
	-22,9,-20,2,-23,-3,
	2,-1,17,3,24,19,
	-11,1,-24,3,20,20,
	-15,-8,-20,12,-14,-11,
	20,-8,19,-14,-14,9,
	-24,15,-8,-22,9,-4,
	22,0,0,-7,24,-3,
	19,-17,23,-17,1,11,
	-24,3,-16,-18,15,-7,
	10,-10,9,0,23,4,
	9,2,15,9,-20,-23,
	16,11,7,-19,8,23,
	17,-14,21,-17,-18,-14,
	-10,11,-10,-24,2,13,
	24,7,-2,15,-3,11,
	-20,18,-9,21,4,-7,
	-5,-18,-23,10,-6,11,
	9,0,13,-1,-24,-12,
	17,10,1,-1,-23,15,
	24,-8,24,-2,-4,14,
	-18,0,18,6,-24,-23,
	16,-19,11,-9,-18,-1,
	-24,-5,-23,-3,22,15,
	-23,0,-19,-15,9,12,
	3,-11,3,-10,22,-21,
	-3,-21,-14,-15,-20,19,
	22,8,1,-21,-11,23,
	-9,-4,-10,-17,-9,-2,
	15,20,9,20,-24,3,
	-16,19,-19,-16,0,-21,
	17,12,8,9,-12,11,
	19,-23,12,12,2,0,
	24,-11,24,-16,-22,-19,
	18,1,-7,-2,5,-23,
	10,23,2,-21,1,-10,
	15,-24,16,-11,4,1,
	16,-20,14,-13,-6,13,
	12,15,10,6,-12,-20,
	-23,13,-18,-5,21,1,
	-19,-7,-23,-1,-21,-15,
	-21,-9,-18,-8,-7,-15,
	0,18,-2,-17,-12,10,
	-22,-2,19,21,-14,22,
	24,6,18,1,21,-19,
	14,-14,15,12,3,17,
	-14,-3,4,-13,5,12,
	12,22,24,-11,-9,-23,
	0,11,11,9,-20,1,
	12,12,18,16,-4,9,
	19,-2,-23,-17,11,22,
	15,13,22,-4,17,-3,
	-9,11,-13,-2,-14,-5,
	5,-12,14,-4,18,-12,
	21,2,-19,8,-23,8,
	-10,0,8,-17,12,19,
	18,2,10,-18,-21,13,
	13,-3,19,4,-19,1,
	-21,8,-24,-10,-1,-23,
	18,9,9,9,20,3,
	-11,18,-5,-24,-23,3,
	-15,17,-19,-13,12,19,
	13,3,13,-14,-15,-23,
	10,-23,16,-10,-2,16,
	-14,-23,-6,-21,19,-6,
	-5,-14,-2,21,-8,11,
	13,16,15,6,-24,14,
	-21,-8,-1,-21,1,14,
	19,14,9,-19,5,21,
	8,-18,17,-6,12,1,
	-19,14,-21,-11,-13,23,
	12,-3,9,-18,-17,-14,
	12,21,19,16,-11,19,
	-23,-3,14,22,-24,20,
	-7,13,-18,-5,-18,-23,
	-22,-9,-10,-15,-18,16,
	-5,-12,-4,18,10,-16,
	9,-15,17,-10,6,16,
	16,-1,-20,-6,-7,-20,
	-24,-18,-13,-1,8,-2,
	16,6,18,-22,11,-12,
	4,-15,1,16,-20,6,
	17,2,21,23,15,-4,
	21,-24,13,3,21,8,
	-4,4,-12,-1,19,-7,
	11,5,-2,-12,-21,-13,
	-19,-11,-20,20,-2,-24,
	11,-1,-3,20,-4,-23,
	11,-1,14,2,7,-2,
	-16,-1,-17,23,-8,-14,
	21,6,10,-15,19,-24,
	15,12,1,-23,-6,11,
	-19,19,-18,17,0,-8,
	0
};

void _LATCH(const int start, const int thread_stride, const uint8_t* const __restrict image, const int stride, std::vector<KeyPoint>& keypoints, uint64_t* const __restrict descriptors) {
	for (int i = start; i < start + thread_stride; ++i) {
		uint8_t* const __restrict desc = reinterpret_cast<uint8_t*>(descriptors + (i << 3));
		const KeyPoint pt = keypoints[i];
		const __m128 ptx = _mm_set_ps1(pt.x);
		const __m128 pty = _mm_set_ps1(pt.y);
		const __m128 sin_theta = _mm_set_ps1(sin(pt.angle));
		const __m128 cos_theta = _mm_set_ps1(cos(pt.angle));
		const __m128 scale = _mm_div_ps(_mm_set_ps1(pt.scale), _mm_set_ps1(7.0f));
		const float* __restrict triplet = triplets;
		for (int fragment = 0; fragment < 64; ++fragment) {
			desc[fragment] = 0;
			for (int bit = 0; bit < 8; ++bit, triplet += 6) {
				__m128 xs = _mm_mul_ps(_mm_loadu_ps(triplet), scale);
				__m128 ys = _mm_mul_ps(_mm_loadu_ps(triplet + 3), scale);
				__m128i xints = _mm_cvtps_epi32(_mm_add_ps(_mm_min_ps(_mm_max_ps(_mm_sub_ps(_mm_mul_ps(xs, cos_theta), _mm_mul_ps(ys, sin_theta)), _mm_set_ps1(-32.0f)), _mm_set_ps1(32.0f)), ptx));
				__m128i yints = _mm_cvtps_epi32(_mm_add_ps(_mm_min_ps(_mm_max_ps(_mm_add_ps(_mm_mul_ps(xs, sin_theta), _mm_mul_ps(ys, cos_theta)), _mm_set_ps1(-32.0f)), _mm_set_ps1(32.0f)), pty));
				const int ax2 = _mm_extract_epi32(xints, 0);
				const int bx2 = _mm_extract_epi32(xints, 1);
				const int cx2 = _mm_extract_epi32(xints, 2);
				const int ay2 = _mm_extract_epi32(yints, 0);
				const int by2 = _mm_extract_epi32(yints, 1);
				const int cy2 = _mm_extract_epi32(yints, 2);
				__m128i accum = _mm_setzero_si128();
				for (int patchy = -3; patchy <= 4; ++patchy) {
					const uint8_t* const __restrict im_a = image + stride*(ay2 + patchy) + ax2 - 3;
					const uint8_t* const __restrict im_b = image + stride*(by2 + patchy) + bx2 - 3;
					const uint8_t* const __restrict im_c = image + stride*(cy2 + patchy) + cx2 - 3;
					const __m128i b1 = _mm_cvtepu8_epi32(_mm_loadu_si128(reinterpret_cast<const __m128i*>(im_b)));
					const __m128i b2 = _mm_cvtepu8_epi32(_mm_loadu_si128(reinterpret_cast<const __m128i*>(im_b + 4)));
					__m128i da1 = _mm_sub_epi32(_mm_cvtepu8_epi32(_mm_loadu_si128(reinterpret_cast<const __m128i*>(im_a))), b1);
					da1 = _mm_mullo_epi32(da1, da1);
					__m128i da2 = _mm_sub_epi32(_mm_cvtepu8_epi32(_mm_loadu_si128(reinterpret_cast<const __m128i*>(im_a + 4))), b2);
					da2 = _mm_add_epi32(da1, _mm_mullo_epi32(da2, da2));
					__m128i dc1 = _mm_sub_epi32(_mm_cvtepu8_epi32(_mm_loadu_si128(reinterpret_cast<const __m128i*>(im_c))), b1);
					dc1 = _mm_mullo_epi32(dc1, dc1);
					__m128i dc2 = _mm_sub_epi32(_mm_cvtepu8_epi32(_mm_loadu_si128(reinterpret_cast<const __m128i*>(im_c + 4))), b2);
					accum = _mm_add_epi32(_mm_sub_epi32(da2, _mm_add_epi32(dc1, _mm_mullo_epi32(dc2, dc2))), accum);
				}
				accum = _mm_hadd_epi32(accum, accum);
				desc[fragment] |= ((_mm_extract_epi32(accum, 0) + _mm_extract_epi32(accum, 1)) & 0x80000000) >> (31 - bit);
			}
		}
	}
}

template<bool multithread>
void LATCH(const uint8_t* const __restrict image, const int width, const int height, const int stride, std::vector<KeyPoint>& keypoints, uint64_t* const __restrict descriptors) {
	keypoints.erase(std::remove_if(keypoints.begin(), keypoints.end(), [width, height](const KeyPoint& kp) {return kp.x <= 36 || kp.y <= 36 || kp.x >= width - 36 || kp.y >= height - 36; }), keypoints.end());
	const int sz = static_cast<int>(keypoints.size());
	if (multithread) {
		const int32_t hw_concur = std::min(sz >> 4, static_cast<int32_t>(std::thread::hardware_concurrency()));
		if (hw_concur > 1) {
			std::vector<std::future<void>> fut(hw_concur);
			const int thread_stride = (sz - 1) / hw_concur + 1;
			int i = 0, start = 0;
			for (; i < std::min(sz - 1, hw_concur - 1); ++i, start += thread_stride) {
				fut[i] = std::async(std::launch::async, _LATCH, start, thread_stride, image, stride, std::ref(keypoints), descriptors);
			}
			fut[i] = std::async(std::launch::async, _LATCH, start, sz - start, image, stride, std::ref(keypoints), descriptors);
			for (int j = 0; j <= i; ++j) fut[j].wait();
		}
		else {
			_LATCH(0, sz, image, stride, std::ref(keypoints), descriptors);
		}
	}
	else {
		_LATCH(0, sz, image, stride, std::ref(keypoints), descriptors);
	}
}