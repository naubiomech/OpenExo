package com.example.andriod_app.data

import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext
import org.json.JSONObject
import java.io.BufferedReader
import java.io.InputStreamReader
import java.net.HttpURLConnection
import java.net.URL
import java.net.URLEncoder

//firebase realtime db rest api wrapper.
//project: class-project-25e45
//us-central pattern is "<id>-default-rtdb.firebaseio.com"
//if u create the db in a different region firebase shows a slightly different
//url on the db page (e.g. <id>-default-rtdb.europe-west1.firebasedatabase.app)
private const val DB_BASE = "https://class-project-25e45-default-rtdb.firebaseio.com"

object FirebaseDbApi {

    //save trial under /users/<uid>/trials/<auto_id>
    //POST = firebase generates the key, returns it as "name" in the response
    suspend fun saveTrial(user: AuthUser, trial: TrialRecord): Result<String> = withContext(Dispatchers.IO) {
        val json = JSONObject().apply{
            put("fileName", trial.fileName)
            put("notes", trial.notes)
            put("durationSec", trial.durationSec)
            put("rowCount", trial.rowCount)
            put("markCount", trial.markCount)
            put("timestampMs", trial.timestampMs)
        }.toString()
        val path = "users/${user.uid}/trials"
        return@withContext request("POST", path, json, user.idToken).map { resp ->
            JSONObject(resp).optString("name", "unknown")
        }
    }

    //save user controller settings - basic key/val storage
    suspend fun saveSettings(user: AuthUser, key: String, value: String): Result<Unit> = withContext(Dispatchers.IO){
        val body = JSONObject().put("value", value).put("updatedAt", System.currentTimeMillis()).toString()
        //sanitize key - firebase paths cant have . / # $ [ ] etc
        val safe = key.filter { it.isLetterOrDigit() || it == '_' }
        val path = "users/${user.uid}/settings/$safe"
        return@withContext request("PUT", path, body, user.idToken).map { Unit }
    }

    //list all trials for current user - returns the raw json string so caller can do whatever
    suspend fun listTrials(user: AuthUser): Result<String> = withContext(Dispatchers.IO){
        return@withContext request("GET", "users/${user.uid}/trials", null, user.idToken)
    }

    //one HTTP helper for all 3 verbs we use (POST PUT GET).
    //auth=<idToken> in the query string is how firebase rest api does auth
    private fun request(method: String, path: String, body: String?, idToken: String): Result<String>
    {
        if(DB_BASE.contains("REPLACE")) {
            return Result.failure(Exception("Firebase DB URL not set in FirebaseDb.kt"))
        }
        val tokenParam = "auth=" + URLEncoder.encode(idToken, "UTF-8")
        val urlStr = "$DB_BASE/$path.json?$tokenParam"
        var conn: HttpURLConnection? = null
        try {
            conn = (URL(urlStr).openConnection() as HttpURLConnection).apply{
                requestMethod = method
                connectTimeout = 8000
                readTimeout = 8000
                if(body != null) {
                    doOutput = true
                    setRequestProperty("Content-Type", "application/json")
                }
            }
            if(body != null){
                conn.outputStream.use { it.write(body.toByteArray(Charsets.UTF_8)) }
            }
            val code = conn.responseCode
            val stream = if(code in 200..299) conn.inputStream else conn.errorStream
            val txt = BufferedReader(InputStreamReader(stream)).use{ it.readText() }
            return if(code in 200..299) Result.success(txt)
                else Result.failure(Exception("DB $method failed: HTTP $code: $txt"))
        }
        catch(e: Exception) { return Result.failure(e) }
        finally { conn?.disconnect() }
    }
}
