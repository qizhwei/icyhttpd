#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <httpd_util.h>

CSTATUS RadixTreeCreate(
	OUT RADIX_TREE **Tree)
{
	RADIX_TREE *result;

	result = (RADIX_TREE *)malloc(sizeof(*result));
	if (!result)
		return C_BAD_ALLOC;

	memset(result, 0, sizeof(*result));
	*Tree = result;
	return C_SUCCESS;
}

void RadixTreeDestroy(
	RADIX_TREE *Tree)
{
	int i;

	free(Tree->DeallocPointer);

	for (i = 0; i < RADIX_TREE_ENTRY_MAX; ++i)
		if (Tree->Entries[i].Tree)
			RadixTreeDestroy(Tree->Entries[i].Tree);

	free(Tree);
}

CSTATUS RadixTreeInsert(
	RADIX_TREE *Tree,
	const char *Key,
	void *Context,
	int CaseInsensitive)
{
	CSTATUS status;
	char *dup;
	char *p, *q;
	char c;
	RADIX_TREE_ENTRY *entry;

	if ((p = dup = _strdup(Key)) == NULL)
		return C_BAD_ALLOC;

	if (CaseInsensitive) {
		q = dup;
		while (*q) {
			*q = toupper(*q);
			++q;
		}
	}

	while (1) {
		c = *p++;

		// If the string terminates, add it to the current node
		if (c == '\0') {
			if (Tree->Context != NULL) {
				free(dup);
				return C_ALREADY_EXISTS;
			}

			Tree->Context = Context;
			Tree->DeallocPointer = dup;
			return C_SUCCESS;
		}

		// Find the entry
		entry = &Tree->Entries[--c];
		
		if (entry->Tree == NULL) {

			// If the entry is empty, create a new subtree
			status = RadixTreeCreate(&Tree);
			if (!SUCCESS(status)) {
				free(dup);
				return status;
			}

			// Assign appropriate values
			entry->Tree = Tree;
			entry->Token = p;
			p = "";

		} else {

			q = entry->Token;
			while (1) {
				if (*q == '\0') {
					Tree = entry->Tree;
					break;
				}

				if (*p != *q) {

					// Create and insert a middle node
					status = RadixTreeCreate(&Tree);
					if (!SUCCESS(status)) {
						free(dup);
						return status;
					}

					c = *q - 1;
					*q++ = '\0';
					Tree->Entries[c].Tree = entry->Tree;
					Tree->Entries[c].Token = q;
					entry->Tree = Tree;
					break;
				}

				++p; ++q;
			}
		}
	}
}

void *RadixTreeQuery(
	RADIX_TREE *Tree,
	const char *Key)
{
	char c;
	const char *p, *q;
	RADIX_TREE_ENTRY *entry;

	p = Key;

	while (1) {
		c = *p++;

		if (c == '\0')
			return Tree->Context;

		entry = &Tree->Entries[--c];
		if (entry->Tree == NULL)
			return NULL;

		q = entry->Token;

		while (1) {
			if (*q == '\0') {
				Tree = entry->Tree;
				break;
			}

			if (*p != *q)
				return NULL;

			++p; ++q;
		}
	}
}

void *RadixTreeQueryCaseInsensitive(
	RADIX_TREE *Tree,
	const char *Key)
{
	char c;
	const char *p, *q;
	RADIX_TREE_ENTRY *entry;

	p = Key;

	while (1) {
		c = *p++;

		if (c == '\0')
			return Tree->Context;

		c = toupper(c);
		entry = &Tree->Entries[--c];
		if (entry->Tree == NULL)
			return NULL;

		q = entry->Token;

		while (1) {
			if (*q == '\0') {
				Tree = entry->Tree;
				break;
			}

			if (toupper(*p) != *q)
				return NULL;

			++p; ++q;
		}
	}
}
