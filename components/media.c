/* See LICENSE file for copyright and license details. */
#include <stdio.h>
#include <string.h>

#include "../slstatus.h"
#include "../util.h"

const char *
media_status(const char *unused)
{
	FILE *fp;
	char *p;

	/* Check if any media player is running */
	fp = popen("playerctl status 2>/dev/null", "r");
	if (!fp) {
		return NULL;
	}

	p = fgets(buf, sizeof(buf) - 1, fp);
	pclose(fp);

	if (!p)
		return NULL;

	/* Remove trailing newline */
	if ((p = strrchr(buf, '\n')))
		p[0] = '\0';

	return buf[0] ? buf : NULL;
}

const char *
media_title(const char *unused)
{
	FILE *fp;
	char *p;

	fp = popen("playerctl metadata title 2>/dev/null", "r");
	if (!fp) {
		return NULL;
	}

	p = fgets(buf, sizeof(buf) - 1, fp);
	pclose(fp);

	if (!p)
		return NULL;

	/* Remove trailing newline */
	if ((p = strrchr(buf, '\n')))
		p[0] = '\0';

	return buf[0] ? buf : NULL;
}

const char *
media_artist(const char *unused)
{
	FILE *fp;
	char *p;

	fp = popen("playerctl metadata artist 2>/dev/null", "r");
	if (!fp) {
		return NULL;
	}

	p = fgets(buf, sizeof(buf) - 1, fp);
	pclose(fp);

	if (!p)
		return NULL;

	/* Remove trailing newline */
	if ((p = strrchr(buf, '\n')))
		p[0] = '\0';

	return buf[0] ? buf : NULL;
}

const char *
media_album(const char *unused)
{
	FILE *fp;
	char *p;

	fp = popen("playerctl metadata album 2>/dev/null", "r");
	if (!fp) {
		return NULL;
	}

	p = fgets(buf, sizeof(buf) - 1, fp);
	pclose(fp);

	if (!p)
		return NULL;

	/* Remove trailing newline */
	if ((p = strrchr(buf, '\n')))
		p[0] = '\0';

	return buf[0] ? buf : NULL;
}

const char *
media_time_remaining(const char *unused)
{
	FILE *fp;
	char *p;
	char length_str[32] = "";
	char position_str[32] = "";
	long length_us = 0;  // length in microseconds
	double position_sec = 0.0;  // position in seconds
	double total_sec = 0.0;
	int current_min, current_sec, total_min, total_sec_int;

	/* Get track length in microseconds */
	fp = popen("playerctl metadata mpris:length 2>/dev/null", "r");
	if (fp) {
		if (fgets(length_str, sizeof(length_str) - 1, fp)) {
			if ((p = strrchr(length_str, '\n')))
				p[0] = '\0';
			sscanf(length_str, "%ld", &length_us);
		}
		pclose(fp);
	}

	/* Get current position in seconds */
	fp = popen("playerctl position 2>/dev/null", "r");
	if (fp) {
		if (fgets(position_str, sizeof(position_str) - 1, fp)) {
			if ((p = strrchr(position_str, '\n')))
				p[0] = '\0';
			sscanf(position_str, "%lf", &position_sec);
		}
		pclose(fp);
	}

	/* Format current position / total duration */
	if (length_us > 0) {
		total_sec = length_us / 1000000.0;  // Convert microseconds to seconds

		current_min = (int)position_sec / 60;
		current_sec = (int)position_sec % 60;
		total_min = (int)total_sec / 60;
		total_sec_int = (int)total_sec % 60;

		return bprintf("%d:%02d / %d:%02d", current_min, current_sec, total_min, total_sec_int);
	}

	return NULL;
}

const char *
media(const char *fmt)
{
	FILE *fp;
	char *p;
	char status[32] = "Unknown";
	char title[256] = "";
	char artist[256] = "";

	/* Get status */
	fp = popen("playerctl status 2>/dev/null", "r");
	if (fp) {
		if (fgets(status, sizeof(status) - 1, fp)) {
			if ((p = strrchr(status, '\n')))
				p[0] = '\0';
		}
		pclose(fp);
	}

	/* Only show if playing or paused */
	if (strcmp(status, "Playing") != 0 && strcmp(status, "Paused") != 0) {
		return NULL;
	}

	/* Get title */
	fp = popen("playerctl metadata title 2>/dev/null", "r");
	if (fp) {
		if (fgets(title, sizeof(title) - 1, fp)) {
			if ((p = strrchr(title, '\n')))
				p[0] = '\0';
		}
		pclose(fp);
	}

	/* Get artist */
	fp = popen("playerctl metadata artist 2>/dev/null", "r");
	if (fp) {
		if (fgets(artist, sizeof(artist) - 1, fp)) {
			if ((p = strrchr(artist, '\n')))
				p[0] = '\0';
		}
		pclose(fp);
	}

	/* Format the output based on fmt argument */
	if (fmt && strcmp(fmt, "full") == 0) {
		/* Full format: Artist - Title */
		if (artist[0] && title[0]) {
			return bprintf("%s - %s", artist, title);
		} else if (title[0]) {
			return bprintf("%s", title);
		}
	} else {
		/* Default format: just title */
		return bprintf("%s", title[0] ? title : "No Track");
	}

	return NULL;
}
