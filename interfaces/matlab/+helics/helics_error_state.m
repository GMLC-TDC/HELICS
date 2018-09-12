function v = helics_error_state()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1398230876);
  end
  v = vInitialized;
end
