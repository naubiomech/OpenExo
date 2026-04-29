package com.example.andriod_app.data

import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext
import org.json.JSONObject
import java.io.BufferedReader
import java.io.InputStreamReader
import java.net.HttpURLConnection
import java.net.URL

//firebase web api key (from google-services.json -> client[0].api_key[0].current_key)
//project: class-project-25e45
//note: firebase web api keys are meant to be public in client apps - secruity
//comes from the db rules + auth tokens, not from hiding this string
private const val FIREBASE_API_KEY = "AIzaSyDhSi3WMFCgvyJbrfjJBQZ8f-_PdM9H9aI"

private const val SIGNUP_URL = "https://identitytoolkit.googleapis.com/v1/accounts:signUp"
private const val SIGNIN_URL = "https://identitytoolkit.googleapis.com/v1/accounts:signInWithPassword"

//returns Result<AuthUser> for clean handling in viewmodel
object FirebaseAuthApi
{
    suspend fun signIn(email: String, password: String): Result<AuthUser> = withContext(Dispatchers.IO){
        post(SIGNIN_URL, email, password)
    }

    suspend fun signUp(email: String, password: String): Result<AuthUser> = withContext(Dispatchers.IO) {
        post(SIGNUP_URL, email, password)
    }

    //both endpts take the same json shape so one helper works for both
    private fun post(baseUrl: String, email: String, password: String): Result<AuthUser> {
        if(FIREBASE_API_KEY.startsWith("REPLACE")){
            return Result.failure(Exception("Firebase API key not set in FirebaseAuth.kt"))
        }
        val urlStr = "$baseUrl?key=$FIREBASE_API_KEY"
        val body = JSONObject().apply {
            put("email", email)
            put("password", password)
            put("returnSecureToken", true)
        }.toString()

        var conn: HttpURLConnection? = null
        try {
            conn = (URL(urlStr).openConnection() as HttpURLConnection).apply{
                requestMethod = "POST"
                doOutput = true
                connectTimeout = 8000
                readTimeout = 8000
                setRequestProperty("Content-Type", "application/json")
            }
            conn.outputStream.use { it.write(body.toByteArray(Charsets.UTF_8)) }

            val code = conn.responseCode
            //pick err stream when http code is not 2xx so we can read the firebase err json
            val stream = if(code in 200..299) conn.inputStream else conn.errorStream
            val resp = BufferedReader(InputStreamReader(stream)).use{ it.readText() }
            val json = JSONObject(resp)

            if(code !in 200..299){
                val errMsg = json.optJSONObject("error")?.optString("message") ?: "HTTP $code"
                return Result.failure(Exception(prettifyAuthError(errMsg)))
            }
            val user = AuthUser(
                  uid = json.getString("localId"),
                  email = json.optString("email", email),
                  idToken = json.getString("idToken")
            )
            return Result.success(user)
        }
        catch(e: Exception){
            return Result.failure(e)
        } finally {
            conn?.disconnect()
        }
    }

    //firebase returns codes like EMAIL_EXISTS, INVALID_PASSWORD etc.
    //just turn em into something readble for the ui. add more cases if you hit one.
    private fun prettifyAuthError(msg: String): String {
        if(msg.contains("EMAIL_EXISTS")) return "An account with that email already exists"
        if(msg.contains("INVALID_LOGIN_CREDENTIALS") || msg.contains("INVALID_PASSWORD"))
            return "Wrong email or password"
        if(msg.contains("EMAIL_NOT_FOUND")) return "No account found with that email"
        if(msg.contains("WEAK_PASSWORD")) return "Password too weak (must be 6+ chars)"
        if(msg.contains("TOO_MANY_ATTEMPTS")) return "Too many tries, wait a min"
        //fallthru - just show the raw err
        return msg
    }
}
