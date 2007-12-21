#include "Context.h"
extern "C" {
#include <openssl/ssl.h>
}
#include "Exceptions/SSL.h"
#include "Private/OpenSSL.h"
#include "Details/GetPasswords.h"

#define SCORPION_CONTEXT_DEFAULT_CIPHER_LIST "ALL:!ADH:!LOW:!EXP:!MD5:@STRENGTH"

namespace Scorpion
{
	namespace
	{
		int passwordCallbackFunction__(char * buf, int size, int rwflag, void * user_data)
		{
			if (user_data)
			{
				Details::GetPasswords * get_passwords((Details::GetPasswords*)user_data);
				std::string password;
				assert(size > 0);
				if (rwflag == 0)
					password = get_passwords->getDecryptionPassword(size - 1);
				else
				{
					assert(rwflag == 1);
					password = get_passwords->getEncryptionPassword(size - 1);
				}
				/* Note: it is up to the callback to assure that the size of the string 
				 * it returns is smaller than or equal to the maximum size we gave it. 
				 * We only do an assertion here because it is a *logic* error if the 
				 * function didn't do its job. This is therefore not a vulnerability 
				 * in Scorpion/Arachnida. */
				assert(static_cast< int >(password.size()) < size);
				std::copy(password.begin(), password.end(), buf);
				buf[password.size()] = 0;
				return static_cast< int >(password.size());
			}
			else
				return 0;
		}
	}

	Context::Context(int flags, unsigned int certificate_verification_depth)
		: flags_(flags),
		  ssl_context_(new ::SSL_CTX *(0), freeSSLContext_),
		  password_getter_(0),
		  certificate_verification_depth_(certificate_verification_depth)
	{ /* no-op */ }

	Context::Context(const Context & context)
		: flags_(context.flags_),
		  server_certicate_name_(context.server_certicate_name_),
		  ssl_context_(context.ssl_context_),
		  password_getter_(context.password_getter_),
		  certificate_chain_filename_(context.certificate_chain_filename_),
		  private_key_filename_(context.private_key_filename_),
		  trusted_ca_certificates_filename_(context.trusted_ca_certificates_filename_),
		  trusted_ca_certificates_directory_name_(context.trusted_ca_certificates_directory_name_),
		  certificate_verification_depth_(context.certificate_verification_depth_)
	{ /* no-op */ }

	Context::~Context() throw()
	{ /* no-op */ }

	Context & Context::swap(Context & context) throw()
	{
		std::swap(flags_, context.flags_);
		std::swap(server_certicate_name_, context.server_certicate_name_);
		std::swap(ssl_context_, context.ssl_context_);
		std::swap(password_getter_, context.password_getter_);
		std::swap(certificate_chain_filename_, context.certificate_chain_filename_);
		std::swap(private_key_filename_, context.private_key_filename_);
		std::swap(trusted_ca_certificates_filename_, context.trusted_ca_certificates_filename_);
		std::swap(trusted_ca_certificates_directory_name_, context.trusted_ca_certificates_directory_name_);
		std::swap(certificate_verification_depth_, context.certificate_verification_depth_);
		return *this;
	}

	void Context::setServerCertificateFilename(const boost::filesystem::path & server_cert)
	{
		copyOnWrite();
		server_certicate_name_ = server_cert;
	}

	void Context::setCertificateChainFilename(const boost::filesystem::path & certificate_chain_filename)
	{
		copyOnWrite();
		certificate_chain_filename_ = certificate_chain_filename;
	}

	void Context::setTrustedCACertificatesFilename(const boost::filesystem::path & trusted_ca_certificates_filename)
	{
		copyOnWrite();
		trusted_ca_certificates_filename_ = trusted_ca_certificates_filename;
	}

	void Context::setTrustedCACertificatesDirectoryName(const boost::filesystem::path & trusted_ca_certificates_directory_name)
	{
		copyOnWrite();
		trusted_ca_certificates_directory_name_ = trusted_ca_certificates_directory_name;
	}

	void Context::setPrivateKeyFilename(const boost::filesystem::path & private_key_filename)
	{
		copyOnWrite();
		private_key_filename_ = private_key_filename;
	}
	
	void Context::setFlags(int flags)
	{
		copyOnWrite();
		flags_ = flags;
	}

	::SSL_CTX * Context::getContext() const
	{
		assert(ssl_context_);
		if (!*ssl_context_)
		{
			SSL_METHOD * ssl_method(0);
			switch (flags_ & use_all_transports__)
			{
			case use_sslv2__ : // use SSLv2 only
				// TODO: log a warning here to say that this is a bad idea!
				ssl_method = SSLv2_method();
				break;
			case use_sslv3__ :
				ssl_method = SSLv3_method();
				break;
			case use_tlsv1__ : 
				ssl_method = TLSv1_method();
				break;
			case use_secure_transports__ :
				// we'll set a flag later to not allow SSLv2
			default :
				ssl_method = SSLv23_method(); 
				break;
			}
			SCORPION_PRIVATE_OPENSSL_EXEC(*ssl_context_ = ::SSL_CTX_new(ssl_method), *ssl_context_, Exceptions::SSL::ContextAllocationError);
			if (password_getter_)
			{
				::SSL_CTX_set_default_passwd_cb(*ssl_context_, passwordCallbackFunction__);
				::SSL_CTX_set_default_passwd_cb_userdata(*ssl_context_, password_getter_);
			}
			else
			{ /* use the default password getter */ }
			if (!trusted_ca_certificates_filename_.empty() || !trusted_ca_certificates_directory_name_.empty())
			{
				SCORPION_PRIVATE_OPENSSL_EXEC(int rc(::SSL_CTX_load_verify_locations(*ssl_context_, trusted_ca_certificates_filename_.empty() ? 0 : trusted_ca_certificates_filename_.string().c_str(), trusted_ca_certificates_directory_name_.empty() ? 0 : trusted_ca_certificates_directory_name_.string().c_str())), rc == 1, Exceptions::SSL::SSLContextSetupError);
			}
			else
			{ /* no private key */ }
			if (flags_ & use_openssl_default_trusted_ca_certificates__)
			{
				SCORPION_PRIVATE_OPENSSL_EXEC(int rc(::SSL_CTX_set_default_verify_paths(*ssl_context_)), rc == 1, Exceptions::SSL::SSLContextSetupError);
			}
			else
			{ /* don't load the default trusted CA certificates */ }
			if (!certificate_chain_filename_.empty())
			{
				SCORPION_PRIVATE_OPENSSL_EXEC(int rc(::SSL_CTX_use_certificate_chain_file(*ssl_context_, certificate_chain_filename_.string().c_str())), rc == 1, Exceptions::SSL::SSLContextSetupError);
			}
			else
			{ /* certificate chain file */ }
			if (!private_key_filename_.empty())
			{
				SCORPION_PRIVATE_OPENSSL_EXEC(int rc(::SSL_CTX_use_PrivateKey_file(*ssl_context_, private_key_filename_.string().c_str(), SSL_FILETYPE_PEM)), rc == 1, Exceptions::SSL::SSLContextSetupError);
			}
			else
			{ /* no private key */ }
			if (!server_certicate_name_.empty())
			{
				SCORPION_PRIVATE_OPENSSL_EXEC(int rc(::SSL_CTX_use_certificate_file(*ssl_context_, server_certicate_name_.string().c_str(), SSL_FILETYPE_PEM)), rc == 1, Exceptions::SSL::SSLContextSetupError);
			}
			else
			{ /* no private key */ }
			
			int verification_flags;
			switch (flags_ & _verification_mask__)
			{
			case 0 :
				verification_flags = SSL_VERIFY_NONE;
				break;
			case use_peer_certificate_verification__ :
				verification_flags = SSL_VERIFY_PEER;
				break;
			case fail_if_no_peer_certificate__ : 
				verification_flags = SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT;
				break;
			case verify_client_certificate_only_once__ :
				verification_flags = SSL_VERIFY_PEER | SSL_VERIFY_CLIENT_ONCE;
				break;
			case _verification_mask__ :
				verification_flags = SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT | SSL_VERIFY_CLIENT_ONCE;
				break;
			default :
				/* Note: this could happen if the user doesn't use the enumeration for 
				 * the flags but uses hard-coded values in stead. That would be a bug 
				 * on the user's side. */
				assert(("This should never happen", 0));
			}
			::SSL_CTX_set_verify(*ssl_context_, verification_flags, 0);
			::SSL_CTX_set_verify_depth(*ssl_context_, certificate_verification_depth_);

			/* this code may seem a it obscure, but as there are 512 possible combinations 
			 * for the work-around flags, it is simpler to have nine ternary ops than one 
			 * switch. */
			int option_flags(0);
			option_flags |= (flags_ & work_around_microsoft_session_id_bug__) ? SSL_OP_MICROSOFT_SESS_ID_BUG : 0;
			option_flags |= (flags_ & work_around_netscape_challenge_bug__) ? SSL_OP_NETSCAPE_CHALLENGE_BUG : 0;
			option_flags |= (flags_ & work_around_netscape_reuse_cipher_change_bug__) ? SSL_OP_NETSCAPE_REUSE_CIPHER_CHANGE_BUG : 0;
			option_flags |= (flags_ & work_around_sslref2_reuse_cert_type_bug__) ? SSL_OP_SSLREF2_REUSE_CERT_TYPE_BUG : 0;
			option_flags |= (flags_ & work_around_microsoft_big_sslv3_buffer_bug__) ? SSL_OP_MICROSOFT_BIG_SSLV3_BUFFER : 0;
			option_flags |= (flags_ & work_around_msie_sslv2_rsa_padding_bug__) ? SSL_OP_MSIE_SSLV2_RSA_PADDING : 0;
			option_flags |= (flags_ & work_around_ssleay_080_client_dh_bug__) ? SSL_OP_SSLEAY_080_CLIENT_DH_BUG : 0;
			option_flags |= (flags_ & work_around_tls_d5_bug__) ? SSL_OP_TLS_D5_BUG : 0;
			option_flags |= (flags_ & work_around_tls_block_padding_bug__) ? SSL_OP_TLS_BLOCK_PADDING_BUG : 0;
			option_flags |= (flags_ & work_around_tls_rollback_bug__) ? SSL_OP_TLS_ROLLBACK_BUG : 0;
			option_flags |= (flags_ & work_around_netscape_ca_dn_bug__) ? SSL_OP_NETSCAPE_CA_DN_BUG : 0;
			option_flags |= (flags_ & work_around_netscape_demo_cipher_change_bug__) ? SSL_OP_NETSCAPE_DEMO_CIPHER_CHANGE_BUG : 0;
			/* We now set the options to remove support for SSL versions and TLSv1 and remove certain features */
			option_flags |= (flags_ & dont_insert_empty_fragments__) ? SSL_OP_DONT_INSERT_EMPTY_FRAGMENTS : 0;
			option_flags |= (flags_ & use_dh_only_once__) ? SSL_OP_SINGLE_DH_USE : 0;
			option_flags |= (flags_ & use_ephemeral_rsa__) ? SSL_OP_EPHEMERAL_RSA : 0;
			option_flags |= (flags_ & use_server_cipher_preferences__) ? SSL_OP_CIPHER_SERVER_PREFERENCE : 0;
			option_flags |= (flags_ & enable_pkcs1_check_1__) ? SSL_OP_PKCS1_CHECK_1 : 0;
			option_flags |= (flags_ & enable_pkcs1_check_2__) ? SSL_OP_PKCS1_CHECK_2 : 0;
			option_flags |= (flags_ & use_sslv2__) ? 0 : SSL_OP_NO_SSLv2;
			option_flags |= (flags_ & use_sslv3__) ? 0 : SSL_OP_NO_SSLv3;
			option_flags |= (flags_ & use_tlsv1__) ? 0 : SSL_OP_NO_TLSv1;
			option_flags |= (flags_ & no_session_resumption_negociation__) ? SSL_OP_NO_SESSION_RESUMPTION_ON_RENEGOTIATION : 0;
#ifdef SSL_OP_NO_TICKET
			option_flags |= (flags_ & no_tickets__) ? SSL_OP_NO_TICKET : 0;
#endif
			::SSL_CTX_set_options(*ssl_context_, option_flags);

			int mode_flags(0);
			mode_flags |= (flags_ & enable_partial_write__) ? SSL_MODE_ENABLE_PARTIAL_WRITE: 0;
			mode_flags |= (flags_ & enable_auto_retry__) ? SSL_MODE_AUTO_RETRY : 0;
			SSL_CTX_set_mode(*ssl_context_, mode_flags);

			SCORPION_PRIVATE_OPENSSL_EXEC(int rc(::SSL_CTX_set_cipher_list(*ssl_context_, SCORPION_CONTEXT_DEFAULT_CIPHER_LIST)), rc == 1, Exceptions::SSL::SSLContextSetupError);
			/* Handle ephemeral RSA keys HERE!! */

		}
		else
		{ /* we already have a context */ }
		assert(*ssl_context_);
		return *ssl_context_;
	}

	void Context::setPasswordGetter(Details::GetPasswords * password_getter)
	{
		copyOnWrite();
		password_getter_ = password_getter;
	}

	void Context::setCertificateVerificationDepth(unsigned int certificate_verification_depth)
	{
		copyOnWrite();
		certificate_verification_depth_ = certificate_verification_depth;
	}

	void Context::copyOnWrite()
	{
		/* Because we use a lazy allocation of the SSL context - we allocate it the first time 
		 * someone asks for an actual context - we can implement a copy-on-write mechanism by 
		 * simply dropping our own reference to the context whenever we write something.  */
		if (*ssl_context_)
			ssl_context_.reset(new ::SSL_CTX *(0), freeSSLContext_);
		else
		{ /* no need */ }
	}

	/*static */void Context::freeSSLContext_(::SSL_CTX ** ssl_context) throw()
	{
		::SSL_CTX_free(*ssl_context);
		delete ssl_context;
	}
}

