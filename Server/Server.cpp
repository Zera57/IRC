/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hapryl <hapryl@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/01/04 17:08:52 by zera              #+#    #+#             */
/*   Updated: 2022/01/22 20:04:37 by hapryl           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

ClientService				Server::_clientService = ClientService();
Parser						Server::_parser = Parser();
ConnectionsService			Server::_connectionsService = ConnectionsService();


Server::Server(ServerSettings * settings) :
	_serverSettings(settings),
	_serverSocket(ServerSocket(_serverSettings->getPort())),
	_fdMax(0) {	}

void	Server::run () {
	std::cout << "Starting..." << _serverSocket.getSocket() << std::endl;
	_fdMax = std::max(_serverSocket.getSocket(), _fdMax);
	while (true)
	{
		setFds(_readFds, _writeFds);
		std::cout << "select" << std::endl;
		if (select(_fdMax + 1, &_readFds, &_writeFds, NULL, NULL) == -1) {
			perror("select");
			_exit(4);
		}
		checkStatusReadFd(_readFds, _writeFds);
	}
}

void	Server::setFds(fd_set &readFds, fd_set &writeFds) {
	FD_ZERO(&readFds);
	FD_ZERO(&writeFds);
	_connectionsService.setFds(readFds, writeFds);
	FD_SET(_serverSocket.getSocket(), &readFds);
	_clientService.setFds(writeFds);
}


void	Server::checkStatusReadFd(fd_set &readFds, fd_set &writeFds) {
	if (FD_ISSET(_serverSocket.getSocket(), &readFds))
		connectEvent();
	for (int fd = 4; fd <= _fdMax; fd++) {
		if (FD_ISSET(fd, &readFds)) {
			readEvent(fd);
		}
		if (FD_ISSET(fd, &writeFds)) {
			sendEvent(fd);
		}
	}
}

void Server::connectEvent() {
	try {
		int		socket = _serverSocket.accept();
		_connectionsService.addConnection(socket);
		_connectionsService.addResponse(socket, Message::toServerResponse("REGISTER or LOGIN", SuccessType()));
		_fdMax = std::max(_fdMax, socket);
		std::cout << "Connection accepted " << socket << std::endl;
	} catch(std::exception &e) {
		perror("socket accept");
	}
}

void Server::disconnectEvent(int fd) {
	_connectionsService.disconnect(fd);
	_clientService.disconnectClient(fd);
	std::cout << "Client disconnected " << fd << std::endl;
}

void Server::readEvent(int fd) {
	char			buffer[BUFFER_SIZE + 1];
	int nDataLenght = recv(fd, buffer, BUFFER_SIZE, 0);
	if (nDataLenght < 1) {
		disconnectEvent(fd);
		return ;
	}
	buffer[nDataLenght] = 0;
	
	std::string rawRq = _connectionsService.addRequest(fd, buffer);
	if (rawRq.empty()) {
		return ;
	}
	if (_connectionsService.getTypeConnection(fd) == Connection::NONE) {
		connectionReadEvent(fd, rawRq);
	} else if (_connectionsService.getTypeConnection(fd) == Connection::CLIENT) {
		clientReadEvent(fd, rawRq);
	}

}

void	Server::connectionReadEvent(int fd, std::string &rawRq) {
	UID		uid = UID(atoll(_serverSettings->getPort().c_str()), fd, 0);
	try {
		ClientRequest *clientRequest = _parser.generateClientRequest(rawRq, uid);
		if (clientRequest->getCommand() == Commands::REGISTR || 
			clientRequest->getCommand() == Commands::LOGIN) {
			Response *resp = _clientService.checkToExecute(clientRequest);
			if (resp->getCommandStatus() == Commands::SUCCESS) {
				_connectionsService.setTypeConnection(fd, Connection::CLIENT);
				if (clientRequest->getCommand() == Commands::REGISTR) {
					_clientService.registrClient(fd, resp);
				} else if (clientRequest->getCommand() == Commands::LOGIN) {
					_clientService.loginClient(fd, resp);
				}
			} else if (resp->getCommandStatus() == Commands::FAIL || resp->getCommandStatus() == Commands::ERROR) {
				if (clientRequest->getCommand() == Commands::REGISTR) {
					_connectionsService.addResponse(fd,
					 Message::toServerResponse("invalid arguments or this user already exists", FailType()));
				} else if (clientRequest->getCommand() == Commands::LOGIN) {
					_connectionsService.addResponse(fd,
					 Message::toServerResponse("invalid arguments or this user already logged in", FailType()));
				}
			}
			delete resp;
		} else {
			_connectionsService.addResponse(fd,
			 Message::toServerResponse("you need to /LOGIN or /REGISTER", FailType()));
		}
		delete clientRequest;
	} catch (std::exception &e) {
		_connectionsService.addResponse(fd, e.what());
	}
}

void		Server::clientReadEvent(int fd, std::string &rawRq) {
	try {
		UID		uid = UID(atoll(_serverSettings->getPort().c_str()), _clientService.getUserId(fd), _clientService.getIdRequest(fd));
		ClientRequest *clientRequest = _parser.generateClientRequest(rawRq, uid);
		Response *response =_clientService.checkToExecute(clientRequest);
		if (response->getCommandStatus() == Commands::SUCCESS) {
			if (clientRequest->getCommand() == Commands::QUIT) {
				disconnectEvent(fd);
			} else if (clientRequest->getCommand() == Commands::KILL) {
				std::cout << response->getArguments()[0] << std::endl;
				disconnectEvent(atoll(response->getArguments()[0].c_str()));
			} else if (clientRequest->getCommand() == Commands::OPER &&
				 response->getArguments()[0] != _serverSettings->getPassword()) {
				response->setStatus(Commands::FAIL);
			}
		}
		_clientService.execute(response);
		delete clientRequest;
		delete response;
	} catch (std::exception &e) {

	}
}

void Server::sendEvent(int sock) {
	std::cout << "Send " << sock << " type " << _connectionsService.getTypeConnection(sock) << std::endl;
	if (_connectionsService.getTypeConnection(sock) == Connection::CLIENT) {
		_clientService.sendResponseToClient(sock);
	}
	_connectionsService.sendMsg(sock);
	FD_CLR(sock, &_writeFds);
}