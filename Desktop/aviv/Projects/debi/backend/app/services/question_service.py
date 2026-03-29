from app.core.database import supabase
from app.models.schemas import QuestionResponse
import uuid

def get_next_question(user_id: str) -> QuestionResponse:
    """
    Selects the next question according to specific logic.
    Currently returns mock data.
    """
    # TODO: Implement complex selection logic using Supabase
    # For example:
    # if supabase is not None:
    #     response = supabase.table("questions").select("*").execute()
    
    return QuestionResponse(
        question_id=str(uuid.uuid4()),
        text="What is your favorite programming language?",
        metadata={"category": "Technology", "difficulty": "Easy", "is_placeholder": True}
    )

# logic - 
# 1. get user profile
# 2. get user answered questions
# 3. get user question pool
# 4. 
