#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <sys/stat.h>
#include <openssl/sha.h>

void computeBigFileSHA256 ( std::string fileName, std::string & sha256HashString, long long & fileSize );

int main ( int argc, char**argv ) {
	std::cout << "Program: SHA256 Photo Renamer, V0.1" << std::endl
			  << "Author : Daniel C Scearce" << std::endl
			  << "License: Apache License Version 2.0" << std::endl << std::endl;

	std::string fileName = "";
	std::string sha256HashString = "";
	long long fileSize = 0;

	// get file name
	if ( argc != 2 ) {
		std::cerr << "Error: You must specify a file name as the first command line argument."
			<< std::endl;
#ifdef _WIN32
		system ( "PAUSE" );
#endif
		return 1;
	}
	fileName = argv [ 1 ];
	// end of get file name

	try {
		computeBigFileSHA256 ( fileName, sha256HashString, fileSize );
	}
	catch ( const std::exception & e ) {
		std::cerr << e.what() << std::endl;
	}

	std::cout << "File Name: " << fileName << std::endl;
	std::cout << "File Size: " << fileSize << std::endl;
	std::cout << "SHA256   : " << sha256HashString << std::endl;

#ifdef _WIN32
	system ( "PAUSE" );
#endif
	return 0;
}

#ifdef _WIN32
#define stat64 _stat64
#endif // _WIN32
// todo: clean up unused variables, implement unix support
void computeBigFileSHA256 ( std::string fileName, std::string & sha256HashString, long long & fileSize ) {
	// open and hash file
	struct stat64 statBuffer;
	const unsigned long fileBufferSize = 1024 * 100;
	char fileBuffer [ fileBufferSize ];
	unsigned char sha256Hash [ SHA256_DIGEST_LENGTH ];
	std::stringstream sha256HashStringBuffer;
	SHA256_CTX sha256Context;
	SHA256_Init ( &sha256Context );
	unsigned int packetSize;
	float percentComplete = 0.0f;
	long long fileSizeRemaining;

	// get file statistics, allowing for files with zero bytes
	if ( stat64 ( fileName.c_str ( ), &statBuffer ) != 0 ) {
#ifdef _WIN32
		char errorMessageCString [ 64 ];
		strerror_s ( errorMessageCString, sizeof errorMessageCString, errno );
		std::string errorMessage = errorMessageCString;
		// todo: implement unix support
#endif // _WIN32
		throw std::exception ( "Error: File could not be accessed." );
	}
	fileSize = statBuffer.st_size;
	fileSizeRemaining = fileSize;
	// end of get file statistics

	// attempt to open file only after checked for existance
	std::ifstream currentFile ( fileName, std::ifstream::binary );
	while ( !currentFile.eof ( ) ) {
		currentFile.read ( fileBuffer, fileBufferSize );
		packetSize = fileBufferSize;
		if ( fileSizeRemaining < fileBufferSize ) {
			packetSize = ( unsigned int ) fileSizeRemaining;
		}

		percentComplete = 100 - ( ( float ) fileSizeRemaining / ( float ) fileSize * 100 );

		fileSizeRemaining = fileSizeRemaining - fileBufferSize;
		SHA256_Update ( &sha256Context, fileBuffer, packetSize );
	}
	// close file
	currentFile.close ( );

	SHA256_Final ( sha256Hash, &sha256Context );
	for ( int i = 0; i < SHA256_DIGEST_LENGTH; i++ ) {
		sha256HashStringBuffer << std::hex << static_cast< unsigned >( sha256Hash [ i ] );
	}
	sha256HashString = sha256HashStringBuffer.str ( );
}
