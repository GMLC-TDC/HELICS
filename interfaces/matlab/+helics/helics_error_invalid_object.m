function v = helics_error_invalid_object()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1398230855);
  end
  v = vInitialized;
end
