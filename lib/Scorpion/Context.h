#ifndef _scorpion_context_h
#define _scorpion_context_h

#include "Details/prologue.h"
#include <boost/filesystem/path.hpp>
#include <boost/shared_ptr.hpp>

typedef struct ssl_ctx_st SSL_CTX;
namespace Scorpion
{
	namespace Details
	{
		class GetPasswords;
	}
	class SCORPION_API Context
	{
	public :
		enum {
			use_sslv2__										= 0x00000001,
			use_sslv3__										= 0x00000002,
			use_tlsv1__										= 0x00000004,
			use_secure_transports__							= 0x00000006,
			use_all_transports__							= 0x00000007,
			use_openssl_default_trusted_ca_certificates__	= 0x00000008,
			use_peer_certificate_verification__				= 0x00000010,
			fail_if_no_peer_certificate__					= 0x00000030,	// has the same effect as use_peer_certificate_verification__ on a client
			verify_client_certificate_only_once__			= 0x00000050,	// has the same effect as use_peer_certificate_verification__ on a client
			_verification_mask__							= 0x00000070,
			work_around_microsoft_session_id_bug__			= 0x00000080,
			work_around_netscape_challenge_bug__			= 0x00000100,
			work_around_netscape_reuse_cipher_change_bug__	= 0x00000200,
			work_around_sslref2_reuse_cert_type_bug__		= 0x00000400,
			work_around_microsoft_big_sslv3_buffer_bug__	= 0x00000800,
			work_around_msie_sslv2_rsa_padding_bug__		= 0x00001000,
			work_around_ssleay_080_client_dh_bug__			= 0x00002000,
			work_around_tls_d5_bug__						= 0x00004000,
			work_around_tls_block_padding_bug__				= 0x00008000,
			work_around_all_bugs__							= 0x0608ff80,
			no_session_resumption_negociation__				= 0x00010000,	// ignored on a client connection
			no_tickets__									= 0x00020000,	// ignored on certain versions of OpenSSL
			dont_insert_empty_fragments__					= 0x00040000,
			work_around_tls_rollback_bug__					= 0x00080000,
			use_dh_only_once__								= 0x00100000,
			use_ephemeral_rsa__								= 0x00200000,	// WARNING: this breaks the SSL protocol
			use_server_cipher_preferences__					= 0x00400000,
			enable_pkcs1_check_1__							= 0x00800000,
			enable_pkcs1_check_2__							= 0x01000000,
			work_around_netscape_ca_dn_bug__				= 0x02000000,
			work_around_netscape_demo_cipher_change_bug__	= 0x04000000,
			enable_partial_write__							= 0x10000000,
			enable_auto_retry__								= 0x20000000,

			default_options__								= use_secure_transports__ | use_peer_certificate_verification__ | fail_if_no_peer_certificate__ | enable_auto_retry__,
			insecure_default_options__						= use_sslv2__ | use_secure_transports__ | enable_auto_retry__ | work_around_all_bugs__,
		};

		Context(int flags = default_options__, unsigned int certificate_verification_depth = 0);
		Context(const Context & context);
		~Context() throw();

		Context & operator=(Context context) { return swap(context); }
		Context & swap(Context & context) throw();

		void setServerCertificateFilename(const boost::filesystem::path & server_cert);
		boost::filesystem::path getServerCertificateFilename() const { return server_certicate_name_; }

		void setCertificateChainFilename(const boost::filesystem::path & certificate_chain_filename);
		boost::filesystem::path getCertificateChainFilename() const { return certificate_chain_filename_; }

		void setTrustedCACertificatesFilename(const boost::filesystem::path & trusted_ca_certificates_filename);
		boost::filesystem::path getTrustedCACertificatesFilename() const { return trusted_ca_certificates_filename_; }
		void setTrustedCACertificatesDirectoryName(const boost::filesystem::path & trusted_ca_certificates_directory_name);
		boost::filesystem::path getTrustedCACertificatesDirectoryName() const { return trusted_ca_certificates_directory_name_; }

		void setPrivateKeyFilename(const boost::filesystem::path & private_key_filename);
		boost::filesystem::path getPrivateKeyFilename() const { return private_key_filename_; }

		void setFlags(int flags);
		int getFlags() const throw() { return flags_; }

		::SSL_CTX * getContext() const;

		void setPasswordGetter(Details::GetPasswords * password_getter);

		void setCertificateVerificationDepth(unsigned int certificate_verification_depth);

	private :
		void copyOnWrite();
		static void freeSSLContext_(::SSL_CTX ** ssl_context) throw();

		int flags_;
		boost::filesystem::path server_certicate_name_;
		mutable boost::shared_ptr< ::SSL_CTX * > ssl_context_;
		Details::GetPasswords * password_getter_;
		boost::filesystem::path certificate_chain_filename_;
		boost::filesystem::path private_key_filename_;
		boost::filesystem::path trusted_ca_certificates_filename_;
		boost::filesystem::path trusted_ca_certificates_directory_name_;
		unsigned int certificate_verification_depth_;
	};
}

#endif
