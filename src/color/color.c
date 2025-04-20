#include <stdint.h>
#include <stdio.h>
#include "color.h"

void rgb_to_hsl(uint32_t argb, float* h, float* s, float* l, uint8_t* a) {
	uint8_t r = (argb >> 16) & 0xFF;
	uint8_t g = (argb >> 8) & 0xFF;
	uint8_t b = argb & 0xFF;

	float r_norm = r / 255.0f;
	float g_norm = g / 255.0f;
	float b_norm = b / 255.0f;

	float max_val = r_norm;
	float min_val = r_norm;
	float delta;
	
	/* Find largest color value */
	if (g_norm > max_val) {
		max_val = g_norm;
	}
	if (b_norm > max_val) {
		max_val = b_norm;
	}
	
	/* Find smallest color value */
	if (g_norm < min_val) {
		min_val = g_norm;
	}
	if (b_norm < min_val) {
		min_val = b_norm;
	}

	/* Save the alpha value */
	*a = (argb >> 24) & 0xFF;
	
	/* Calculate lightness */
	*l = (max_val + min_val) / 2.0f;

	delta = max_val - min_val;

	/* If grayscale */
	if(delta == 0.0f) {
		*s = 0.0f;
		*h = 0.0f;
	} else {
		*s = (*l < 0.5f) ? (delta / (max_val + min_val)) : (delta / (2.0f - max_val - min_val));

		if(max_val == r_norm) {
			*h = (g_norm - b_norm) / delta + (g_norm < b_norm ? 6.0f : 0.0f);
		} else if(max_val == g_norm) {
			*h = (b_norm - r_norm) / delta + 2.0f;
		} else {
			*h = (r_norm - g_norm) / delta + 4.0f;
		}
		*h *= 60.0f;
	}
}

void hsl_to_rgb(float h, float s, float l, uint8_t a, uint32_t* argb) {
	float q, p, h_norm, r_norm, g_norm, b_norm, r_clamp, b_clamp, g_clamp;
	uint8_t r, g, b;

	/* If grayscale */
	if (s == 0.0f) {
		r_norm = g_norm = b_norm = l;
	} else {
		q = (l < 0.5f) ? (l * (1.0f + s)) : (l + s - l * s);
		p = 2.0f * l - q;
		h_norm = h / 360.0f;

		r_norm = hue_to_rgb(p, q, h_norm + 1.0f / 3.0f);
		g_norm = hue_to_rgb(p, q, h_norm);
		b_norm = hue_to_rgb(p, q, h_norm - 1.0f / 3.0f);
	}

	/* Clamp the color values between 0 and 1 and convert to int */
	r_clamp = (r_norm < 0.0f) ? 0.0f : ((r_norm > 1.0f) ? 1.0f : r_norm);
	g_clamp = (g_norm < 0.0f) ? 0.0f : ((g_norm > 1.0f) ? 1.0f : g_norm);
	b_clamp = (b_norm < 0.0f) ? 0.0f : ((b_norm > 1.0f) ? 1.0f : b_norm);
	
	/* Add + 0.5f for correct rounding */
	r = (uint8_t)(r_clamp * 255 + 0.5f);
	g = (uint8_t)(g_clamp * 255 + 0.5f);
	b = (uint8_t)(b_clamp * 255 + 0.5f);

	/* Combine colors with alpha */
	*argb = ((uint32_t)a << 24) | ((uint32_t)r << 16) | ((uint32_t)g << 8) | (uint32_t)b;
}

float hue_to_rgb(float p, float q, float t) {
	/* Map HSL cylinder onto RGB cube */
	if(t < 0.0f) {
		t += 1.0f;
	}
	if(t > 1.0f) {
		t -= 1.0f;
	}
	if(t < 1.0f / 6.0f) {
		return p + (q - p) * 6.0f * t;
	}
	if(t < 1.0f / 2.0f) {
		return q;
	}
	if(t < 2.0f / 3.0f) {
		return p + (q - p) * (2.0f / 3.0f - t) * 6.0f;
	}
	return p;
}
