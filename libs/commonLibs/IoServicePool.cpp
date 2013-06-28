#include "stdafx.h"

#include "IoServicePool.hpp"

CIoServicePool* CIoServicePool::pService = NULL;

CIoServicePool::CIoServicePool( std::size_t pool_size )
	: m_nIoServiceCount( 0 ), m_dxNextSpIoService( 0 )
{
	// pool ũ�⸸ŭ io_service ����
	for( std::size_t idx = 0; idx < pool_size; ++idx )
	{
		tySpIoService io_service( new boost::asio::io_service );
		tySpIoServiceWork io_service_work( new boost::asio::io_service::work( *io_service ) );

		m_vecSpIoService.push_back( io_service );
		m_vecSpIoServiceWork.push_back( io_service_work );
	}
}

CIoServicePool::~CIoServicePool()
{
	if( CIoServicePool::pService != NULL )
		delete CIoServicePool::pService;
	CIoServicePool::pService = NULL;
}

CIoServicePool* CIoServicePool::getInstance( std::size_t pool_size /* = 0 */ )
{
	if( CIoServicePool::pService == NULL )
		CIoServicePool::pService = new CIoServicePool( pool_size == 0 ? 4 : pool_size );

	return CIoServicePool::pService;
}

void CIoServicePool::run()
{
	// io_service ���� ������ �����带 �����Ѵ�. 
	std::vector< boost::shared_ptr< boost::thread > >	vecThread;
	for( std::size_t idx = 0; idx < m_vecSpIoService.size(); ++idx )
	{
		boost::shared_ptr< boost::thread > th( new boost::thread( 
			boost::bind( &boost::asio::io_service::run, m_vecSpIoService[ idx ] ) ) );

		vecThread.push_back( th );
	}

	// ������ ���Ḧ ��ٸ����� �Ʒ� �ڵ带 �ּ�����
	// 	for( std::size_t idx = 0; idx < vecThread.size(); ++idx )
	// 		vecThread[ idx ]->timed_join( boost::posix_time::seconds(1) );
}

void CIoServicePool::stop()
{
	// io_service �� run �Լ��� work ���翩�ο� ������� ����ǵ��� ��. 
	for( std::size_t idx = 0; idx < m_vecSpIoService.size(); ++idx )
		m_vecSpIoService[ idx ]->stop();
}

boost::asio::io_service& CIoServicePool::getNextIoService()
{
	// ������ ����� io_service �� round_robin ����� ����Ͽ� ����
	boost::asio::io_service& io_service = *m_vecSpIoService[ m_dxNextSpIoService ];

	InterlockedIncrement( (LONG *)&m_dxNextSpIoService );

	if( m_dxNextSpIoService == m_vecSpIoService.size() ) 
		m_dxNextSpIoService = 0;

	return io_service;
}
