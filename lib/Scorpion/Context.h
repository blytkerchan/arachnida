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
	/** A security context for use with server or client connections.
	 * Using an instance of this object allows you to secure any connection created by 
	 * Spin (you have to use the context to create the connection, of course). Note, 
	 * though, that the security of such a connection is limited by the security of the
	 * settings you use to create the context. There are literally millions of combinations
	 * possible, some of which reduce security rather than enhance it, but some of which
	 * you need to use in order to talk to existing software, such as HTTPS servers or
	 * clients.
	 *
	 * For any connection to work, you should at least allow the use of one of the three
	 * supported transports: SSLv2, SSLv3 or TLSv1. */
	class SCORPION_API Context
	{
	public :
		//! Flags to be used when creating the context
		enum Flags 
		{
			use_sslv2__										= 0x00000001,	///< Allow the use of SSLv2 - \em not recommended
			use_sslv3__										= 0x00000002,	///< Allow the use of SSLv3
			use_tlsv1__										= 0x00000004,	///< Allow the use of TLSv1
			use_secure_transports__							= 0x00000006,	///< Allow the use of SSLv3 and TLSv1 - convenience combination
			use_all_transports__							= 0x00000007,	///< Allow the use of any protocol - convenience combination
			use_openssl_default_trusted_ca_certificates__	= 0x00000008,	///< Allow the use of OpenSSL's default trusted CA certificates - recommended only on safe machines where you need these certificates
			use_peer_certificate_verification__				= 0x00000010,	///< Ask the server/client for a certificate and verify it. If not specified, no verification will be performed against trusted CA certificates
			fail_if_no_peer_certificate__					= 0x00000030,	///< Fail if the peer does not have a valid certificate -  has the same effect as use_peer_certificate_verification__ on a client
			verify_client_certificate_only_once__			= 0x00000050,	///< verify a client's certificate only once - has the same effect as use_peer_certificate_verification__ on a client
#ifndef DOXYGEN_GENERATING
			_verification_mask__							= 0x00000070,
#endif
			work_around_microsoft_session_id_bug__			= 0x00000080,	///< www.microsoft.com - when talking SSLv2, if session-id reuse is performed, the session-id passed back in the server-finished message is different from the one decided upon.
			work_around_netscape_challenge_bug__			= 0x00000100,	///< Netscape-Commerce/1.12, when talking SSLv2, accepts a 32 byte challenge but then appears to only use 16 bytes when generating the encryption keys.  Using 16 bytes is ok but it should be ok to use 32.  According to the SSLv3 spec, one should use 32 bytes for the challenge when operating in SSLv2/v3 compatibility mode, but as mentioned above, this breaks this server so 16 bytes is the way to go.
			work_around_netscape_reuse_cipher_change_bug__	= 0x00000200,	///< ssl3.netscape.com:443, first a connection is established with RC4-MD5.  If it is then resumed, we end up using DES-CBC3-SHA.  It should be RC4-MD5 according to 7.6.1.3, 'cipher_suite'.\nNetscape-Enterprise/2.01 (https://merchant.netscape.com) has this bug.  It only really shows up when connecting via SSLv2/v3 then reconnecting via SSLv3. The cipher list changes....\nTry connecting with a cipher list of just DES-CBC-SHA:RC4-MD5.  For some weird reason, each new connection uses RC4-MD5, but a re-connect tries to use DES-CBC-SHA.  So netscape, when doing a re-connect, always takes the first cipher in the cipher list.
			work_around_sslref2_reuse_cert_type_bug__		= 0x00000400,	///< no information about this bug
			work_around_microsoft_big_sslv3_buffer_bug__	= 0x00000800,	///< no information about this bug
			work_around_msie_sslv2_rsa_padding_bug__		= 0x00001000,	///< As of OpenSSL 0.9.7h and 0.9.8a, this option has no effect.
			work_around_ssleay_080_client_dh_bug__			= 0x00002000,	///< no information about this bug
			work_around_tls_d5_bug__						= 0x00004000,	///< no information about this bug
			work_around_tls_block_padding_bug__				= 0x00008000,	///< no information about this bug
			work_around_all_bugs__							= 0x0608ff80,	///< Convenience combinartion to enable all work-arounds
			no_session_resumption_negociation__				= 0x00010000,	///< When performing renegotiation as a server, always start a new session (i.e., session resumption requests are only accepted in the initial handshake).  This option is not needed for clients.\nIgnored on a client connection
			no_tickets__									= 0x00020000,	///< Normally clients and servers will, where possible, transparently make use of RFC4507bis tickets for stateless session resumption if extension support is explicitly set when OpenSSL is compiled.\nIf this option is set this functionality is disabled and tickets will not be used by clients or servers.\nIgnored on certain versions of OpenSSL
			dont_insert_empty_fragments__					= 0x00040000,	///< Disables a countermeasure against a SSL 3.0/TLS 1.0 protocol vulnerability affecting CBC ciphers, which cannot be handled by some broken SSL implementations.  This option has no effect for connections using other ciphers.
			work_around_tls_rollback_bug__					= 0x00080000,	///< Disable version rollback attack detection.\nDuring the client key exchange, the client must send the same information about acceptable SSL/TLS protocol levels as during the first hello. Some clients violate this rule by adapting to the server's answer. (Example: the client sends a SSLv2 hello and accepts up to SSLv3.1=TLSv1, the server only understands up to SSLv3. In this case the client must still use the same SSLv3.1=TLSv1 announcement. Some clients step down to SSLv3 with respect to the server's answer and violate the version rollback protection.)
			use_dh_only_once__								= 0x00100000,	///< Always create a new key when using temporary/ephemeral DH parameters (see SSL_CTX_set_tmp_dh_callback(3)).  This option must be used to prevent small subgroup attacks, when the DH parameters were not generated using "strong" primes (e.g. when using DSA-parameters, see dhparam(1)).  If "strong" primes were used, it is not strictly necessary to generate a new DH key during each handshake but it is also recommended.  SSL_OP_SINGLE_DH_USE should therefore be enabled whenever temporary/ephemeral DH parameters are used.
			use_ephemeral_rsa__								= 0x00200000,	///< Always use ephemeral (temporary) RSA key when doing RSA operations (see SSL_CTX_set_tmp_rsa_callback(3)).  According to the specifications this is only done, when a RSA key can only be used for signature operations (namely under export ciphers with restricted RSA keylength). By setting this option, ephemeral RSA keys are always used. This option breaks compatibility with the SSL/TLS specifications and may lead to interoperability problems with clients and should therefore never be used. Ciphers with EDH (ephemeral Diffie-Hellman) key exchange should be used instead. \warning this breaks the SSL protocol
			use_server_cipher_preferences__					= 0x00400000,	///< When choosing a cipher, use the server's preferences instead of the client preferences. When not set, the SSL server will always follow the clients preferences. When set, the SSLv3/TLSv1 server will choose following its own preferences. Because of the different protocol, for SSLv2 the server will send its list of preferences to the client and the client chooses.
			enable_pkcs1_check_1__							= 0x00800000,	///< no information about this feature
			enable_pkcs1_check_2__							= 0x01000000,	///< no information about this feature
			work_around_netscape_ca_dn_bug__				= 0x02000000,	///< If we accept a netscape connection, demand a client cert, have a non-self-signed CA which does not have its CA in netscape, and the browser has a cert, it will crash/hang. Works for 3.x and 4.xbeta
			work_around_netscape_demo_cipher_change_bug__	= 0x04000000,	///< no information about this bug
			enable_partial_write__							= 0x10000000,	///< allow write(n) to return some x with 0 <= x <= n
			enable_auto_retry__								= 0x20000000,	///< automatically retry if interrupted due to SSL protocol

			//! Convenience combination: secure defaults
			default_options__								= use_secure_transports__ | use_peer_certificate_verification__ | fail_if_no_peer_certificate__ | enable_auto_retry__,
			//! Convenience combination: less secure defaults
			insecure_default_options__						= use_sslv2__ | use_secure_transports__ | enable_auto_retry__ | work_around_all_bugs__,
		};

		/** Construct a context with the given flags and certificate verification depth.
		 * The actual SSL context is created on-demand and is shared by all SSL connections created 
		 * with the same context and the same settings. The Context class uses a copy-on-write mechanism
		 * to allow for this.
		 * \param flags the flags to use to create the SSL context, when needed.
		 * \param certificate_verification_depth the number of certificates in a PKI to check to find a trusted CA. */
		Context(int flags = default_options__, unsigned int certificate_verification_depth = 0);
		//! Copy constructor with copy-on-write supported value-type semantics
		Context(const Context & context);
		~Context() throw();

		//! Assignment operator with copy-on-write supported value-type semantics
		Context & operator=(Context context) { return swap(context); }
		//! Non-throwing swap
		Context & swap(Context & context) throw();

		//! Set the name of the file that contains the server certificate. Ignored by client connections created with this context
		void setServerCertificateFilename(const boost::filesystem::path & server_cert);
		//! Get the name of the file that contains the server certificate.
		boost::filesystem::path getServerCertificateFilename() const { return server_certicate_name_; }

		/** Set the name of the file that contains the certificate chain.
		 * Loads a certificate chain from file into ctx. The certificates must be in 
		 * PEM format and must be sorted starting with the subject's certificate (actual
		 * client or server certificate), followed by intermediate CA certificates if
		 * applicable, and ending at the highest level (root) CA. */
		void setCertificateChainFilename(const boost::filesystem::path & certificate_chain_filename);
		//! Get the name of the file that contains the certificate chain.
		boost::filesystem::path getCertificateChainFilename() const { return certificate_chain_filename_; }

		/** Set the name of the file with trusted CA certificates.
		 * It points to a file of CA certificates in PEM format. The file can contain several CA certificates.
		 * Before, between, and after the certificates text is allowed which can be used e.g. for 
		 * descriptions of the certificates.
		 *
		 * The CAfile is processed on execution of the SSL_CTX_load_verify_locations() function, 
		 * which is called when the context is retrieved for the first time. This is also when 
		 * the password callbacks will be called - if ever - for the CA certificates. */
		void setTrustedCACertificatesFilename(const boost::filesystem::path & trusted_ca_certificates_filename);
		//! Get the name of the file with trusted CA certificates.
		boost::filesystem::path getTrustedCACertificatesFilename() const { return trusted_ca_certificates_filename_; }
		/** Set the name of a directory with trusted CA certificates in it.
		 * It points to a directory containing CA certificates in PEM format. The files each contain one CA 
		 * certificate. The files are looked up by the CA subject name hash value, which must hence be 
		 * available.  If more than one CA certificate with the same name hash value exist, the extension 
		 * must be different (e.g. 9d66eef0.0, 9d66eef0.1 etc).
		 *
		 * The search is performed in the ordering of the extension number, regardless of other properties of 
		 * the certificates.  Use the c_rehash utility to create the necessary links.
		 *
		 * The certificates in the provided path are only looked up when required, e.g. when building the 
		 * certificate chain or when actually performing the verification of a peer certificate. 
		 *
		 * When looking up CA certificates, the OpenSSL library will first search the certificates in the 
		 * file provided to setTrustedCACertificatesFilename, then those in the path provided to 
		 * setTrustedCACertificatesDirectoryName. Certificate matching is done based on the subject name,
		 * the key identifier (if present), and the serial number as taken from the certificate to be 
		 * verified. If these data do not match, the next certificate will be tried. If a first certificate 
		 * matching the parameters is found, the verification process will be performed; no other certificates 
		 * for the same parameters will be searched in case of failure.
		 *
		 * In server mode, when requesting a client certificate, the server must send the list of CAs of 
		 * which it will accept client certificates. This list is not influenced by the contents of the file
		 * of which the name is provided to setTrustedCACertificatesFilename, or the directory of which 
		 * the name is provided to setTrustedCACertificatesDirectoryName. */
		void setTrustedCACertificatesDirectoryName(const boost::filesystem::path & trusted_ca_certificates_directory_name);
		//! Get the name of a directory with trusted CA certificates in it.
		boost::filesystem::path getTrustedCACertificatesDirectoryName() const { return trusted_ca_certificates_directory_name_; }

		/** Set the name of the file containing the private key.
		 * Adds the first private key found in the file to the context, when created. The file
		 * must be formatted in PEM format. */
		void setPrivateKeyFilename(const boost::filesystem::path & private_key_filename);
		//! Get the name of the file containing the private key.
		boost::filesystem::path getPrivateKeyFilename() const { return private_key_filename_; }

		//! Set the flags of the context, and thus the future connections to be created
		void setFlags(int flags);
		//! Get the currently set flags
		int getFlags() const throw() { return flags_; }

		//! Get an SSL context. This will allocate one if need be
		::SSL_CTX * getContext() const;

		//! Set the functions to be called to get a password when needed
		void setPasswordGetter(Details::GetPasswords * password_getter);
		//! Set the password verification depth - 0 == no limit
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
