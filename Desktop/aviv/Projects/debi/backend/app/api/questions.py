from fastapi import APIRouter
from app.models.schemas import QuestionResponse
from app.services.question_service import get_next_question

router = APIRouter()

@router.get("/next", response_model=QuestionResponse)
def get_next(user_id: str):
    """
    Returns the next question for a given user.
    """
    return get_next_question(user_id)
