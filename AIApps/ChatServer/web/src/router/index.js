import { createRouter, createWebHistory } from 'vue-router'
import EntryView from '../views/EntryView.vue'
import MenuView from '../views/MenuView.vue'
import ChatView from '../views/ChatView.vue'
import UploadView from '../views/UploadView.vue'
import NotFoundView from '../views/NotFoundView.vue'

const router = createRouter({
  history: createWebHistory(),
  routes: [
    { path: '/', name: 'entry', component: EntryView },
    { path: '/menu', name: 'menu', component: MenuView },
    { path: '/chat', name: 'chat', component: ChatView },
    { path: '/upload', name: 'upload', component: UploadView },
    { path: '/:pathMatch(.*)*', name: 'not-found', component: NotFoundView }
  ]
})

export default router