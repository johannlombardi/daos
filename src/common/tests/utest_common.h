/**
 * (C) Copyright 2019 Intel Corporation.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * GOVERNMENT LICENSE RIGHTS-OPEN SOURCE SOFTWARE
 * The Government's rights to use, modify, reproduce, release, perform, display,
 * or disclose this software are subject to the terms of the Apache License as
 * provided in Contract No. B609815.
 * Any reproduction of computer software, computer software documentation, or
 * portions thereof marked with this legend must also reproduce the markings.
 *
 * Helper library for low level tests (e.g. btree/evtree)
 */
#include <daos/mem.h>

#define UTEST_POOL_NAME_MAX	255

struct utest_context;

/** Create pmem context for unit testing.  This creates a pool and a root
 *  object of specified sizes.  Note that the root object should be
 *  retrieved with utest_utx2root* rather than pmemobj_root as the real
 *  root type is internal to the helper library.
 *
 *  \param	name[IN]	The path of the pool
 *  \param	pool_size[IN]	The size of the pool in bytes
 *  \param	root_size[IN]	The size of the root object
 *  \param	utx[OUT]
 *
 *  \return 0 on success, error otherwise
 */
int utest_pmem_create(const char *name, size_t pool_size, size_t root_size,
		      struct utest_context **utx);

/** Create vmem context for unit testing.  This allocates a context and
 *  a root object of the specified size.  This isn't wholly necessary but
 *  ensures that we have a common interface whether we are using vmem or
 *  pmem.
 *
 *  \param	root_size[IN]	The size of the root object
 *  \param	utx[OUT]
 *
 *  \return 0 on success, error otherwise
 */
int utest_vmem_create(size_t root_size, struct utest_context **utx);

/** Destroy a context and free any associated resources (e.g. memory,
 *  pmemobj pool, files)
 *
 *  \param	utx[IN]	The context to destroy
 *
 *  \return 0 on success, error otherwise
 */
int utest_utx_destroy(struct utest_context *utx);

/** Retrieve a pointer to the root object in a context
 *
 *  \param	utx[IN]	The context to query
 *
 *  \return	Pointer to root
 */
void *utest_utx2root(struct utest_context *utx);

/** Retrieve an mmid pointer to the root object in a context
 *
 *  \param	utx[IN]	The context to query
 *
 *  \return	mmid pointer to root
 */
umem_id_t utest_utx2rootmmid(struct utest_context *utx);

/** Initialization callback for object allocation API.  If the context
 *  is a pmem context, this will be invoked inside a transaction.  The
 *  pointer will already be added to the transaction but it is up to the
 *  user to add any additional memory modified.
 *
 *  \param	ptr[IN, OUT]	The pointer to initialize
 *  \param	size[IN]	The size of the allocation
 *  \param	cb_arg[IN]	Argument passed in by user
 */
typedef void (*utest_init_cb)(void *ptr, size_t size, const void *cb_arg);

/** Allocate an object and, optionally, initialize it.  If the context
 *  is a pmem context, this will be done in a transaction.
 *
 *  \param	utx[IN]		The context
 *  \param	mmid[OUT]	The allocated pointer
 *  \param	size[IN]	The size to allocate
 *  \param	cb[IN]		Optional initialization callback
 *  \param	cb_arg[IN]	Optional argument to pass to initialization
 *
 *  \return 0 on success, error otherwise
 */
int utest_alloc(struct utest_context *utx, umem_id_t *mmid, size_t size,
		utest_init_cb cb, const void *cb_arg);

/** Free an object
 *
 *  \param	utx[IN]		The context
 *  \param	mmid[IN]	The allocated pointer to free
 *
 *  \return 0 on success, error otherwise
 */
int utest_free(struct utest_context *utx, umem_id_t mmid);

/** Free a typed object
 *
 *  \param	utx[IN]		The context
 *  \param	tmmid[IN]	The allocated pointer to free
 *
 *  \return 0 on success, error otherwise
 */
#define utest_free_typed(utx, tmmid) utest_free(utx, (tmmid).oid)

/** Get the umem_instance for a context
 *
 *  \param	utx[IN]		The context
 *
 *  \return umem_instance
 */
struct umem_instance *utest_utx2umm(struct utest_context *utx);

/** Get the umem_attr for a context
 *
 *  \param	utx[IN]		The context
 *
 *  \return The umem_attr
 */
struct umem_attr *utest_utx2uma(struct utest_context *utx);

/** Helper macro to convert an offset to an mmid
 *  \param	utx[IN]		The context
 *  \param	offset[IN]	The offset from start of pool
 *
 *  \return The mmid to the memory
 */
#define utest_off2mmid(utx, offset)					\
	(								\
	{								\
		umem_id_t	__ummid;				\
		uint64_t	__pool_uuid;				\
									\
		__pool_uuid = umem_get_uuid(utest_utx2umm(utx));	\
		__ummid.pool_uuid_lo = __pool_uuid;			\
		__ummid.off = (offset);					\
		__ummid;						\
	}								\
	)

/** Helper macro to convert an offset to an direct pointer
 *  \param	utx[IN]		The context
 *  \param	offset[IN]	The offset from start of pool
 *
 *  \return The pointer to the memory
 */
#define utest_off2ptr(utx, offset)					\
	umem_id2ptr(utest_utx2umm(utx), utest_off2mmid(utx, offset))
