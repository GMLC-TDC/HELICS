function v = halted()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 6);
  end
  v = vInitialized;
end
