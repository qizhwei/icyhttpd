#include "FcgiHandler.h"
#include "Utility.h"
#include "Exception.h"
#include "fcpool.h"

using namespace Httpd;

namespace
{
	void FcRequestBeginCallback(void *completion, int error)
	{
		Dispatcher &d = Dispatcher::Instance();
		d.CompleteApc(completion, reinterpret_cast<void *>(error));
	}

	void FcRequestIoCompletion(void *completion, fc_ssize_t size)
	{
		Dispatcher &d = Dispatcher::Instance();
		d.CompleteApc(completion, reinterpret_cast<void *>(size));
	}
}

namespace Httpd
{
	FcObject::FcObject()
		: u(NULL)
	{}

	FcObject::FcObject(const FcObject &o)
		: u(o.u ? fc_duplicate(o.u) : NULL)
	{}

	FcObject::FcObject(void *u)
		: u(u)
	{}

	FcObject &FcObject::operator =(FcObject &o)
	{
		this->Pointer(o.u ? fc_duplicate(o.u) : NULL);
		return o;
	}

	FcObject::~FcObject()
	{
		this->Pointer(NULL);
	}

	void FcObject::Pointer(void *u)
	{
		if (this->u) {
			fc_close(this->u);
		}
		this->u = u;
	}

	void *FcObject::Pointer()
	{
		return u;
	}

	FcPool::FcPool(const char *cmdLine, int queueLength, int maxInstances,
			int idleTime, int maxRequests)
	{
		fc_pool_t *pool = fc_create_pool(cmdLine, queueLength, maxInstances,
			idleTime, maxRequests);
		if (pool == NULL) {
			throw SystemException();
		}
		this->Pointer(pool);
	}

	FcRequest::FcRequest()
	{
		fc_request_t *req = fc_create_request();
		if (req == NULL) {
			throw SystemException();
		}
		this->Pointer(req);
	}

	void FcRequest::Begin(FcPool *pool)
	{
		Dispatcher &d = Dispatcher::Instance();
		int error = reinterpret_cast<int>(
			d.InvokeApc([=, &d](void *completion)->void
		{
			if (fc_begin_request(
				reinterpret_cast<fc_request_t *>(this->Pointer()),
				reinterpret_cast<fc_pool_t *>(pool->Pointer()),
				&FcRequestBeginCallback, completion)) {
				d.CompleteApc(completion, reinterpret_cast<void *>(-1));
			}
		}));
		if (error) {
			throw SystemException();
		}
	}

	void FcRequest::Abort()
	{
		Dispatcher &d = Dispatcher::Instance();
		d.InvokeApc([=, &d](void *completion)->void
		{
			fc_abort_request(reinterpret_cast<fc_request_t *>(this->Pointer()));
			d.CompleteApc(completion, NULL);
		});
	}

	UInt32 FcRequest::Read(char *buffer, UInt32 size)
	{
		Dispatcher &d = Dispatcher::Instance();
		fc_ssize_t result = reinterpret_cast<fc_ssize_t>(
			d.InvokeApc([=, &d](void *completion)->void
		{
			if (fc_read_request(
				reinterpret_cast<fc_request_t *>(this->Pointer()),
				buffer, size, &FcRequestIoCompletion, completion)) {
				d.CompleteApc(completion, reinterpret_cast<void *>(-1));
			}
		}));
		if (result < 0) {
			throw SystemException();
		}
		return static_cast<UInt32>(result);
	}

	UInt32 FcRequest::Write(const char *buffer, UInt32 size)
	{
		Dispatcher &d = Dispatcher::Instance();
		fc_ssize_t result = reinterpret_cast<fc_ssize_t>(
			d.InvokeApc([=, &d](void *completion)->void
		{
			if (fc_write_request(reinterpret_cast<fc_request_t *>(this->Pointer()),
				buffer, size, &FcRequestIoCompletion, completion)) {
				d.CompleteApc(completion, reinterpret_cast<void *>(-1));
			}
		}));
		if (result < 0) {
			throw SystemException();
		}
		return static_cast<UInt32>(result);
	}

	UInt32 FcRequest::WriteParam(const char *buffer, UInt32 size)
	{
		Dispatcher &d = Dispatcher::Instance();
		fc_ssize_t result = reinterpret_cast<fc_ssize_t>(
			d.InvokeApc([=, &d](void *completion)->void
		{
			if (fc_write_param_request(reinterpret_cast<fc_request_t *>(this->Pointer()),
				buffer, size, &FcRequestIoCompletion, completion)) {
				d.CompleteApc(completion, reinterpret_cast<void *>(-1));
			}
		}));
		if (result < 0) {
			throw SystemException();
		}
		return static_cast<UInt32>(result);
	}
}
