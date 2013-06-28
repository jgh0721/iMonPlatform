#pragma once

#include <vector>

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

class CIoServicePool
	: boost::noncopyable
{
public:
	~CIoServicePool();

	static CIoServicePool* getInstance( std::size_t pool_size = 0 );

	void	run();
	void	stop();

	boost::asio::io_service& getNextIoService();

private:
	CIoServicePool( std::size_t pool_size );

	// io_service::work 는 io_service 의 run() 함수가 async 등으로 동작할 때 즉시 반환되는 것을 막는 역할을 함
	typedef boost::shared_ptr< boost::asio::io_service >		tySpIoService;
	typedef boost::shared_ptr< boost::asio::io_service::work >	tySpIoServiceWork;

	std::vector< tySpIoService >		m_vecSpIoService;
	std::vector< tySpIoServiceWork >	m_vecSpIoServiceWork;

	// ioService pool 에서 다음번 ioService 의 인덱스
	std::size_t							m_dxNextSpIoService;
	std::size_t							m_nIoServiceCount;

	static CIoServicePool				*pService;
};
