async function request(path, options = {}) {
  const response = await fetch(path, {
    credentials: 'include',
    ...options,
    headers: {
      'Content-Type': 'application/json',
      ...(options.headers || {})
    }
  })

  const contentType = response.headers.get('content-type') || ''
  let data = null

  if (contentType.includes('application/json')) {
    data = await response.json()
  } else {
    data = await response.text()
  }

  return { response, data }
}

export async function login(payload) {
  return request('/login', { method: 'POST', body: JSON.stringify(payload) })
}

export async function register(payload) {
  return request('/register', { method: 'POST', body: JSON.stringify(payload) })
}

export async function logout(payload) {
  return request('/user/logout', { method: 'POST', body: JSON.stringify(payload) })
}

export async function getSessions() {
  return request('/chat/sessions', { method: 'GET' })
}

export async function getHistory(sessionId) {
  return request('/chat/history', { method: 'POST', body: JSON.stringify({ sessionId }) })
}

export async function sendMessage(payload) {
  return request('/chat/send', { method: 'POST', body: JSON.stringify(payload) })
}

export async function sendMessageToNewSession(payload) {
  return request('/chat/send-new-session', { method: 'POST', body: JSON.stringify(payload) })
}

export async function uploadImage(payload) {
  return request('/upload/send', { method: 'POST', body: JSON.stringify(payload) })
}
