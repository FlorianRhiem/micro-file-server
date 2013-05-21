/**
 * MIME type detection based on file names
 *
 * The data in mime.data.c is generated from the file mime.types of the Apache project.
 * @file mime.h
 * @author Florian Rhiem (florian.rhiem@gmail.com)
 *
 */
#ifndef MIME_H
#define MIME_H

/**
 * Finds the MIME (Multipurpose Internet Mail Extensions) type that correspons to \c file_name.
 * @param[in] file_name
 *  The file name.
 * @return
 *  The MIME type, or "application/octet-stream" if the file extension of \c file_name is unknown or if \c file_name is \c NULL.
 * @note
 *  This function works with the string literals from mime.data.c, so the returned string must not be freed.
 */
const char *get_mime_type_for_file_name(const char *file_name);
#endif
