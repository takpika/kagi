#ifdef __linux__
#include <crypt/linux.hpp>
#include <tss2/tss2_esys.h>

#define TPM2_HANDLE_ID 0x03F21D
#define PERSISTENT_HANDLE (TPM2_PERSISTENT_FIRST + TPM2_HANDLE_ID)

#include <tss2/tss2_esys.h>
#include <tss2/tss2_rc.h>
#include <iostream>
#include <cstring>

bool createRSAKeyPair() {
    ESYS_CONTEXT *ctx = nullptr;
    TSS2_RC rc;

    rc = Esys_Initialize(&ctx, NULL, NULL);
    if (rc != TSS2_RC_SUCCESS) {
        const char *msg = Tss2_RC_Decode(rc);
        std::cerr << "Error initializing ESYS: " << msg << std::endl;
        return false;
    }

    TPM2B_AUTH authValue = {0};
    TPM2B_PUBLIC inPublic;
    inPublic.publicArea.type = TPM2_ALG_RSA;
    inPublic.publicArea.nameAlg = TPM2_ALG_SHA256;
    inPublic.publicArea.objectAttributes = (TPMA_OBJECT_USERWITHAUTH | TPMA_OBJECT_SENSITIVEDATAORIGIN |
                                            TPMA_OBJECT_SIGN_ENCRYPT | TPMA_OBJECT_FIXEDTPM |
                                            TPMA_OBJECT_FIXEDPARENT | TPMA_OBJECT_DECRYPT);
    inPublic.publicArea.authPolicy.size = 0;
    inPublic.publicArea.parameters.rsaDetail.symmetric.algorithm = TPM2_ALG_NULL;
    inPublic.publicArea.parameters.rsaDetail.scheme.scheme = TPM2_ALG_NULL;
    inPublic.publicArea.parameters.rsaDetail.keyBits = 2048;
    inPublic.publicArea.parameters.rsaDetail.exponent = 0;
    inPublic.publicArea.unique.rsa.size = 0;
    TPM2B_SENSITIVE_CREATE inSensitive = {
        .sensitive = {
            .userAuth = {0},
            .data = {0},
        },
    };
    TPM2B_DATA outsideInfo = {0};
    TPML_PCR_SELECTION creationPCR = {0};
    TPM2B_PUBLIC *outPublic = NULL;
    TPM2B_CREATION_DATA *creationData = NULL;
    TPM2B_DIGEST *creationHash = NULL;
    TPMT_TK_CREATION *creationTicket = NULL;
    ESYS_TR primaryHandle = ESYS_TR_NONE;

    rc = Esys_CreatePrimary(
        ctx,
        ESYS_TR_RH_OWNER,
        ESYS_TR_PASSWORD,
        ESYS_TR_NONE,
        ESYS_TR_NONE,
        &inSensitive,
        &inPublic,
        &outsideInfo,
        &creationPCR,
        &primaryHandle,
        &outPublic,
        &creationData,
        &creationHash,
        &creationTicket
    );

    if (rc != TSS2_RC_SUCCESS) {
        Esys_Finalize(&ctx);
        const char *msg = Tss2_RC_Decode(rc);
        std::cerr << "Error creating primary key: " << msg << std::endl;
        return false;
    }

    TPM2_HANDLE persistentHandle = PERSISTENT_HANDLE;
    rc = Esys_EvictControl(
        ctx,
        ESYS_TR_RH_OWNER,
        primaryHandle,
        ESYS_TR_PASSWORD,
        ESYS_TR_NONE,
        ESYS_TR_NONE,
        persistentHandle,
        &persistentHandle
    );

    if (rc != TSS2_RC_SUCCESS) {
        Esys_FlushContext(ctx, primaryHandle);
        Esys_Finalize(&ctx);
        const char *msg = Tss2_RC_Decode(rc);
        std::cerr << "Error evicting key: " << msg << std::endl;
        return false;
    }

    std::cout << "Key successfully created and persisted with handle: 0x" << std::hex << persistentHandle << std::endl;

    Esys_FlushContext(ctx, primaryHandle);
    Esys_Finalize(&ctx);

    if (outPublic) {
        free(outPublic);
    }
    if (creationData) {
        free(creationData);
    }
    if (creationHash) {
        free(creationHash);
    }
    if (creationTicket) {
        free(creationTicket);
    }

    return true;
}

bool isRSAKeyPairCreated() {
    ESYS_CONTEXT *ctx = nullptr;
    TSS2_RC rc;

    rc = Esys_Initialize(&ctx, NULL, NULL);
    if (rc != TSS2_RC_SUCCESS) {
        const char *msg = Tss2_RC_Decode(rc);
        std::cerr << "Error initializing ESYS: " << msg << std::endl;
        return false;
    }

    TPM2_HANDLE handle = PERSISTENT_HANDLE;
    ESYS_TR objectHandle = ESYS_TR_NONE;

    rc = Esys_TR_FromTPMPublic(
        ctx,
        handle,
        ESYS_TR_NONE,
        ESYS_TR_NONE,
        ESYS_TR_NONE,
        &objectHandle
    );

    if (rc == TSS2_RC_SUCCESS) {
        std::cout << "RSA key pair exists with handle: 0x" << std::hex << handle << std::endl;

        Esys_TR_Close(ctx, &objectHandle);
        Esys_Finalize(&ctx);
        return true;
    } else if ((rc & ~TSS2_RC_LAYER_MASK) == TPM2_RC_HANDLE) {
        std::cout << "RSA key pair does not exist with handle: 0x" << std::hex << handle << std::endl;
    } else {
        std::cerr << "Error checking for RSA key pair: " << std::hex << rc << std::endl;
    }

    Esys_Finalize(&ctx);
    const char *msg = Tss2_RC_Decode(rc);
    std::cerr << "Error checking for RSA key pair: " << msg << std::endl;
    return false;
}

bool encryptWithRSAKey(std::vector<uint8_t> &data, std::vector<uint8_t> &encryptedData) {
    if (!isRSAKeyPairCreated()) {
        if (!createRSAKeyPair()) {
            return false;
        }
    }

    ESYS_CONTEXT *ctx = nullptr;
    TSS2_RC rc;

    rc = Esys_Initialize(&ctx, NULL, NULL);
    if (rc != TSS2_RC_SUCCESS) {
        const char *msg = Tss2_RC_Decode(rc);
        std::cerr << "Error initializing ESYS: " << msg << std::endl;
        return false;
    }

    TPM2_HANDLE handle = PERSISTENT_HANDLE;
    ESYS_TR objectHandle = ESYS_TR_NONE;

    rc = Esys_TR_FromTPMPublic(
        ctx,
        handle,
        ESYS_TR_NONE,
        ESYS_TR_NONE,
        ESYS_TR_NONE,
        &objectHandle
    );

    if (rc != TSS2_RC_SUCCESS) {
        Esys_Finalize(&ctx);
        const char *msg = Tss2_RC_Decode(rc);
        std::cerr << "Error getting object handle: " << msg << std::endl;
        return false;
    }

    TPM2B_PUBLIC_KEY_RSA inData = {
        .size = data.size(),
    };
    memcpy(inData.buffer, data.data(), data.size());

    TPM2B_PUBLIC_KEY_RSA *outData = NULL;
    TPMT_RSA_DECRYPT inScheme = {
        .scheme = TPM2_ALG_RSAES,
        .details = {.anySig = {.hashAlg = TPM2_ALG_SHA256}}
    };

    rc = Esys_RSA_Encrypt(
        ctx,
        objectHandle,
        ESYS_TR_NONE,
        ESYS_TR_NONE,
        ESYS_TR_NONE,
        &inData,
        &inScheme,
        NULL,
        &outData
    );

    if (rc != TSS2_RC_SUCCESS) {
        Esys_Finalize(&ctx);
        const char *msg = Tss2_RC_Decode(rc);
        std::cerr << "Error encrypting data: " << msg << std::endl;
        return false;
    }

    encryptedData.assign(outData->buffer, outData->buffer + outData->size);

    Esys_Finalize(&ctx);
    return true;
}

bool decryptWithRSAKey(std::vector<uint8_t> &encryptedData, std::vector<uint8_t> &decryptedData) {
    if (!isRSAKeyPairCreated()) {
        if (!createRSAKeyPair()) {
            return false;
        }
    }

    ESYS_CONTEXT *ctx = nullptr;
    TSS2_RC rc;

    rc = Esys_Initialize(&ctx, NULL, NULL);
    if (rc != TSS2_RC_SUCCESS) {
        const char *msg = Tss2_RC_Decode(rc);
        std::cerr << "Error initializing ESYS: " << msg << std::endl;
        return false;
    }

    TPM2_HANDLE handle = PERSISTENT_HANDLE;
    ESYS_TR objectHandle = ESYS_TR_NONE;

    rc = Esys_TR_FromTPMPublic(
        ctx,
        handle,
        ESYS_TR_NONE,
        ESYS_TR_NONE,
        ESYS_TR_NONE,
        &objectHandle
    );

    if (rc != TSS2_RC_SUCCESS) {
        Esys_Finalize(&ctx);
        const char *msg = Tss2_RC_Decode(rc);
        std::cerr << "Error getting object handle: " << msg << std::endl;
        return false;
    }

    TPM2B_PUBLIC_KEY_RSA inData = {
        .size = encryptedData.size(),
    };
    memcpy(inData.buffer, encryptedData.data(), encryptedData.size());

    TPM2B_PUBLIC_KEY_RSA *outData = NULL;
    TPMT_RSA_DECRYPT inScheme = {
        .scheme = TPM2_ALG_RSAES,
        .details = {.anySig = {.hashAlg = TPM2_ALG_SHA256}}
    };

    rc = Esys_RSA_Decrypt(
        ctx,
        objectHandle,
        ESYS_TR_PASSWORD,
        ESYS_TR_NONE,
        ESYS_TR_NONE,
        &inData,
        &inScheme,
        NULL,
        &outData
    );

    if (rc != TSS2_RC_SUCCESS) {
        Esys_Finalize(&ctx);
        const char *msg = Tss2_RC_Decode(rc);
        std::cerr << "Error decrypting data: " << msg << std::endl;
        return false;
    }

    decryptedData.assign(outData->buffer, outData->buffer + outData->size);

    Esys_Finalize(&ctx);
    return true;
}

bool encryptKey(AESData key, RSAEncryptedData &encryptedKey) {
    std::vector<uint8_t> keyData(key.key, key.key + sizeof(key.key));
    std::vector<uint8_t> encryptedData;
    if (!encryptWithRSAKey(keyData, encryptedData)) {
        return false;
    }

    if (encryptedData.size() != sizeof(encryptedKey.key)) {
        std::cerr << "Encrypted key size mismatch" << std::endl;
        return false;
    }

    std::copy(encryptedData.begin(), encryptedData.end(), encryptedKey.key);
    return true;
}

bool decryptKey(RSAEncryptedData key, AESData &decryptedKey) {
    std::vector<uint8_t> encryptedData(key.key, key.key + sizeof(key.key));
    std::vector<uint8_t> decryptedData;
    if (!decryptWithRSAKey(encryptedData, decryptedData)) {
        return false;
    }

    if (decryptedData.size() != sizeof(decryptedKey.key)) {
        std::cerr << "Decrypted key size mismatch" << std::endl;
        return false;
    }

    std::copy(decryptedData.begin(), decryptedData.end(), decryptedKey.key);
    return true;
}

AESData generateAESKey() {
    AESData key;
    ESYS_CONTEXT *ctx = nullptr;
    TSS2_RC rc;

    rc = Esys_Initialize(&ctx, NULL, NULL);
    if (rc != TSS2_RC_SUCCESS) {
        const char *msg = Tss2_RC_Decode(rc);
        std::cerr << "Error initializing ESYS: " << msg << std::endl;
        return key;
    }

    TPM2B_DIGEST *randomBytes = NULL;
    rc = Esys_GetRandom(
        ctx,
        ESYS_TR_NONE,
        ESYS_TR_NONE,
        ESYS_TR_NONE,
        32,
        &randomBytes
    );

    if (rc != TSS2_RC_SUCCESS) {
        Esys_Finalize(&ctx);
        const char *msg = Tss2_RC_Decode(rc);
        std::cerr << "Error generating random bytes: " << msg << std::endl;
        return key;
    }

    memcpy(key.key, randomBytes->buffer, sizeof(key.key));

    Esys_Finalize(&ctx);
    return key;
}

#endif