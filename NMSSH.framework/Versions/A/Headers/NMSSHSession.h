#import "NMSSHSessionDelegate.h"

typedef NS_ENUM(NSInteger, NMSSHSessionHash) {
    NMSSHSessionHashMD5,
    NMSSHSessionHashSHA1
};

typedef NS_ENUM(NSInteger, NMSSHKnownHostStatus) {
    NMSSHKnownHostStatusMatch,
    NMSSHKnownHostStatusMismatch,
    NMSSHKnownHostStatusNotFound,
    NMSSHKnownHostStatusFailure
};

/**
NMSSHSession provides the functionality required to setup a SSH connection
and authorize against it.

In it's simplest form it works like this:

    NMSSHSession *session = [NMSSHSession connectToHost:@"127.0.0.1:22"
                                             withUsername:@"user"];

    if (session.isConnected) {
        NSLog(@"Successfully created a new session");
    }

    [session authenticateByPassword:@"pass"];

    if (session.isAuthorized) {
        NSLog(@"Successfully authorized");
    }
*/
@interface NMSSHSession : NSObject

/// ----------------------------------------------------------------------------
/// @name Setting the Delegate
/// ----------------------------------------------------------------------------

/**
 * The receiver’s `delegate`.
 *
 * The `delegate` is sent messages when content is loading.
 */
@property (nonatomic, weak) id<NMSSHSessionDelegate> delegate;

/// ----------------------------------------------------------------------------
/// @name Initialize a new SSH session
/// ----------------------------------------------------------------------------

/**
 * Shorthand method for initializing a NMSSHSession object and calling connect.
 *
 * @param host The server hostname (a port number can be specified by appending
 *             `@":{portnr}"`
 * @param username A valid username the server will accept
 * @returns NMSSHSession instance
 */
+ (id)connectToHost:(NSString *)host withUsername:(NSString *)username;

/**
 * Shorthand method for initializing a NMSSHSession object and calling connect,
 * (explicitly setting a port number).
 *
 * @param host The server hostname
 * @param port The port number
 * @param username A valid username the server will accept
 * @returns NMSSHSession instance
 */
+ (id)connectToHost:(NSString *)host port:(NSInteger)port withUsername:(NSString *)username;

/**
 * Create and setup a new NMSSH instance.
 *
 * @param host The server hostname (a port number can be specified by appending
 *             `@":{portnr}"`
 * @param username A valid username the server will accept
 * @returns NMSSHSession instance
 */
- (id)initWithHost:(NSString *)host andUsername:(NSString *)username;

/**
 * Create and setup a new NMSSH instance.
 *
 * @param host The server hostname
 * @param port The port number
 * @param username A valid username the server will accept
 * @returns NMSSHSession instance
 */
- (id)initWithHost:(NSString *)host port:(NSInteger)port andUsername:(NSString *)username;

/// ----------------------------------------------------------------------------
/// @name Connection settings
/// ----------------------------------------------------------------------------

/** Full server hostname in the format `@"{hostname}:{port}"`. */
@property (nonatomic, readonly) NSString *host;

/** The server port to connect to. */
@property (nonatomic, readonly) NSNumber *port;

/** Username that will authenticate against the server. */
@property (nonatomic, readonly) NSString *username;

/** Timeout for libssh2 blocking functions. */
@property (nonatomic, strong) NSNumber *timeout;

/** Last session error */
@property (nonatomic, readonly) NSError *lastError;

/**
 * The hash algorithm to use to encode the fingerprint
 * during connection, default value is NMSSHSessionHashMD5.
 */
@property (nonatomic, assign) NMSSHSessionHash fingerprintHash;

/// ----------------------------------------------------------------------------
/// @name Raw libssh2 session and socket reference
/// ----------------------------------------------------------------------------

/** Raw libssh2 session instance. */
@property (nonatomic, readonly, getter = rawSession) LIBSSH2_SESSION *session;

/** Raw session socket. */
@property (nonatomic, readonly) CFSocketRef socket;

/// ----------------------------------------------------------------------------
/// @name Open/Close a connection to the server
/// ----------------------------------------------------------------------------

/**
 * A Boolean value indicating whether the session connected successfully
 * (read-only).
 */
@property (nonatomic, readonly, getter = isConnected) BOOL connected;

/**
 * Connect to the server using the default timeout (10 seconds)
 *
 * @returns Connection status
 */
- (BOOL)connect;

/**
 * Connect to the server.
 *
 * @param timeout The time, in seconds, to wait before giving up.
 * @returns Connection status
 */
- (BOOL)connectWithTimeout:(NSNumber *)timeout;

/**
 * Close the session
 */
- (void)disconnect;

/// ----------------------------------------------------------------------------
/// @name Authentication
/// ----------------------------------------------------------------------------

/**
 * A Boolean value indicating whether the session is successfully authorized
 * (read-only).
 */
@property (nonatomic, readonly, getter = isAuthorized) BOOL authorized;

/**
 * Authenticate by password
 *
 * @param password Password for connected user
 * @returns Authentication success
 */
- (BOOL)authenticateByPassword:(NSString *)password;

/**
 * Authenticate by private key pair
 *
 * Use password:nil when the key is unencrypted
 *
 * @param publicKey Filepath to public key
 * @param privateKey Filepath to private key
 * @param password Password for encrypted private key
 * @returns Authentication success
 */
- (BOOL)authenticateByPublicKey:(NSString *)publicKey
                     privateKey:(NSString *)privateKey
                    andPassword:(NSString *)password;

/**
 * Authenticate by private key pair. First it tries to authenticate with an empty passphrase on the
 * private key. If that fails, |passwordBlock| is called to get the passpharse for the private key
 * and authentication is attempted again.
 *
 *
 * @param publicKey Filepath to public key
 * @param privateKey Filepath to private key
 * @param passwordBlock Returns the passphrase for the private key. Only called if the private key
 *   is encrypted. May return nil to abort authentication.
 * @returns Authentication success
 */
- (BOOL)authenticateByPublicKey:(NSString *)publicKey
                     privateKey:(NSString *)privateKey
          optionalPasswordBlock:(NSString *(^)())passwordBlock;

/**
 * Authenticate by keyboard-interactive using delegate.
 *
 * @returns Authentication success
 */
- (BOOL)authenticateByKeyboardInteractive;

/**
 * Authenticate by keyboard-interactive using block.
 *
 * @param authenticationBlock The block to apply to server requests.
 *     The block takes one argument:
 *
 *     _request_ - Question from server<br>
 *     The block returns a NSString object that represents a valid response
 *     to the given question.
 * @returns Authentication success
 */
- (BOOL)authenticateByKeyboardInteractiveUsingBlock:(NSString *(^)(NSString *request))authenticationBlock;

/**
 * Setup and connect to an SSH agent
 *
 * @returns Authentication success
 */
- (BOOL)connectToAgent;

/**
 * Get supported authentication methods
 *
 * @returns Array of string descripting supported authentication methods
 */
- (NSArray *)supportedAuthenticationMethods;

/**
 * Get the fingerprint of the remote host.
 * The session must be connected to an host.
 *
 * @param hashType The hash algorithm to use to encode the fingerprint
 * @returns The host's fingerprint
 */
- (NSString *)fingerprint:(NMSSHSessionHash)hashType;

/// ----------------------------------------------------------------------------
/// @name Known hosts
/// ----------------------------------------------------------------------------

/**
 * Checks if the hosts's key is recognized. The session must be connected.
 *
 * @returns Known host status for current host.
 */
- (NMSSHKnownHostStatus)knownHostStatus;

/**
 * Adds the current host to the user's known hosts file. It is added to the
 * file unhashed.
 *
 * @returns Success status.
 */
- (BOOL)addCurrentHostToKnownHostsUnhashed;

/**
 * Adds the passed-in host to the user's known hosts file. The hostName may be a
 * numerical IP address or a full name. If it includes a port number, it should
 * be formatted as [host]:port (e.g., @"[example.com]:2222"). If salt is non-nil
 * then hostName must contain a hostname hashed with SHA1 and then
 * base64-encoded.
 *
 * @param hostName The hostname to add.
 * @param salt The base64-encoded salt used for hashing. May be nil.
 * @returns Success status.
 */
- (BOOL)addHostToKnownHosts:(NSString *)hostName
                   withSalt:(NSString *)salt;

/// ----------------------------------------------------------------------------
/// @name Quick channel/sftp access
/// ----------------------------------------------------------------------------

/** Get a pre-configured NMSSHChannel object for the current session (read-only). */
@property (nonatomic, readonly) NMSSHChannel *channel;

/** Get a pre-configured NMSFTP object for the current session (read-only). */
@property (nonatomic, readonly) NMSFTP *sftp;

@end
