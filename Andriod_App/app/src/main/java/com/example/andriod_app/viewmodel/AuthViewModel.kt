package com.example.andriod_app.viewmodel

import androidx.lifecycle.ViewModel
import androidx.lifecycle.viewModelScope
import com.example.andriod_app.data.AuthUser
import com.example.andriod_app.data.FirebaseAuthApi
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.flow.asStateFlow
import kotlinx.coroutines.launch

//handles login / signup / signout via firebase auth rest api
//keeps the user obj in a stateflow so the screens can react when its set
class AuthViewModel : ViewModel()
{

    private val _user = MutableStateFlow<AuthUser?>(null)
    val user: StateFlow<AuthUser?> = _user.asStateFlow()

    //busy = network call in flight (so we can disable buttons + show spinner)
    private val _busy = MutableStateFlow(false)
    val busy: StateFlow<Boolean> = _busy.asStateFlow()

    private val _error = MutableStateFlow<String?>(null)
    val error: StateFlow<String?> = _error.asStateFlow()

    fun clearError() { _error.value = null }

    fun signIn(email: String, password: String, onDone: (Boolean) -> Unit) {
        if(email.isBlank() || password.isBlank()){
            _error.value = "Please fill in all fields"
            onDone(false)
            return
        }
        _busy.value = true
        _error.value = null
        viewModelScope.launch {
            val res = FirebaseAuthApi.signIn(email.trim(), password)
            _busy.value = false
            res.onSuccess {
                _user.value = it
                onDone(true)
            }.onFailure {
                _error.value = it.message ?: "Login failed"
                onDone(false)
            }
        }
    }

    //signUp - same flow as signIn but hits the signup endpoint + extra length check
    //TODO: probably should also validate the email format here but firebase rejects bad ones anyway
    fun signUp(email: String, password: String, onDone: (Boolean) -> Unit) {
        //quick gaurd - bail if anythings empty
        if(email.isBlank() || password.isBlank()) {
            _error.value = "Please fill in all fields"
            onDone(false); return
        }
        //firebase requires 6+ char passwords, fail early so user gets a nicer msg
        if(password.length < 6){
            _error.value = "Password must be at least 6 characters"
            onDone(false); return
        }

        _busy.value = true; _error.value = null

        viewModelScope.launch {
            val res = FirebaseAuthApi.signUp(email.trim(), password)
            _busy.value = false
            if(res.isSuccess){
                _user.value = res.getOrNull()
                onDone(true)
            } else {
                _error.value = res.exceptionOrNull()?.message ?: "Sign up failed"
                onDone(false)
            }
        }
    }

    //local only - no logout endpoint on rest api, just nuke the user obj
    fun signOut(){ _user.value = null }
}
