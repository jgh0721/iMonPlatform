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

	// io_service::work �� io_service �� run() �Լ��� async ������ ������ �� ��� ��ȯ�Ǵ� ���� ���� ������ ��
	typedef boost::shared_ptr< boost::asio::io_service >		tySpIoService;
	typedef boost::shared_ptr< boost::asio::io_service::work >	tySpIoServiceWork;

	std::vector< tySpIoService >		m_vecSpIoService;
	std::vector< tySpIoServiceWork >	m_vecSpIoServiceWork;

	// ioService pool ���� ������ ioService �� �ε���
	std::size_t							m_dxNextSpIoService;
	std::size_t							m_nIoServiceCount;

	static CIoServicePool				*pService;
};
