#ifndef __SVC_FILE_H
#define __SVC_FILE_H

void svc_file();
esp_err_t dump_file(const char *path);

#define EVT_CARD_DETECT		(1 << 0)

#endif